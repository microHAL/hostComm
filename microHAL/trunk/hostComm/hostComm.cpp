/*
 * hostComm.cpp
 *
 *  Created on: 13 cze 2014
 *      Author: Dell
 */

#include "hostComm.h"

namespace microhal {

using namespace diagnostic;
using namespace std::chrono_literals;

HostCommPacket HostComm::pingPacket(HostCommPacket::PING, false, false);
HostCommPacket HostComm::pongPacket(HostCommPacket::PONG, false, false);
//HostCommPacket_ACK HostComm::ACKpacket;

bool HostComm::send(HostCommPacket &packet) {
	std::lock_guard<std::mutex> lock(sendMutex);
	//count up from 0 to 15
	sentCounter = (sentCounter + 1) & 0x0F;
	//
	packet.setNumber(sentCounter);
	packet.calculateCRC();

	if(sentPacktToIODevice(packet) == false){
		log << DEBUG << "Unable to sent packet." << endl;
		return false;
	}

	if (packet.requireACK()) {
		//txPendingPacket = &packet;
		for (uint16_t retransmission = 0; retransmission < maxRetransmissionTry; retransmission++) {
			log << Informational << "Waiting for ACK...";
			if (waitForACK(packet)) {
				log << Informational << "OK" << endl;
				return true;
			}
			log << Informational << "Missing" << endl << "Retransmitting packet." << endl;
			if(sentPacktToIODevice(packet) == false){
				log << DEBUG << "Unable to sent packet." << endl;
				return false;
			}
		}
		//unable to deliver packet
		return false;
	}
	return true;
}

bool HostComm::sentPacktToIODevice(HostCommPacket &packet) {
	//check if packet info data and payload are continous in memory
	if(reinterpret_cast<void*>(&packet.packetInfo + sizeof(HostCommPacket::PacketInfo)) == packet.getDataPtr()){
		const size_t sizeToTransfer = packet.getSize() + sizeof(HostCommPacket::PacketInfo);
		if(ioDevice.write((char*)(&packet.packetInfo), sizeToTransfer) == sizeToTransfer) return true;
	} else {
		if(ioDevice.write((char*)(&packet.packetInfo), sizeof(HostCommPacket::PacketInfo)) !=  sizeof(HostCommPacket::PacketInfo)) return false;
		if(ioDevice.write(packet.getDataPtr<char>(), packet.getSize()) == packet.getSize()) return true;
	}
	return false;
}

bool HostComm::ping(bool waitForResponse) {
	std::lock_guard<std::mutex> guard(sendMutex);

	log << lock << Debug << "HostComm: Sending PING..." << unlock;
	bool status = send(pingPacket);
	if (status == true)
		log << lock << Debug << "OK" << endl << unlock;
		if(waitForResponse == true) {
			log << lock << Debug  << "HostComm: Waiting for PONG..." << endl << unlock;

			if(ackSemaphore.wait(ackTimeout)) {
				if (receivedPacket.getType() == HostCommPacket::PONG) {
					log << lock << Debug << "OK" << endl << unlock;
					return true;
				} else {
					log << lock << Debug << "ERROR" << endl << unlock;
					return false;
				}
			} else {
				log << lock << Informational  << "Unable to receive PONG, semaphore timeout." << endl << unlock;
			}
	}
	return status;
}

bool HostComm::waitForACK(HostCommPacket &packetToACK) {
	if(ackSemaphore.wait(ackTimeout)) {
		HostCommPacket_ACK &ack = static_cast<HostCommPacket_ACK&>(receivedPacket);
		if (ack.isAcknowledged(packetToACK)) {
			return true;
		}
	} else {
		log << Informational << "Unable to receive ACK, semaphore timeout." << endl;
	}
	return false;
}

void HostComm::timeProc() {
	static uint32_t i;
	if(i++ == 500){
		i = 0;
		log << lock << Debug  << "HostComm: timeProc function was called 2000 times." << endl << unlock;
	}

	if (readPacket() == true) {
		log << Debug << "HostComm: got packet." << endl;
		if (receivedPacket.checkCRC() == true) {
			// if need do send ack
			if (receivedPacket.requireACK() == true) {
				//send ack
				ACKpacket.setPacketToACK(receivedPacket);
				if (send(ACKpacket)) {
					log << Debug << "HostComm: ACK sent" << endl;
				} else {
					log << Debug << "HostComm: unable to send ACK."	<< endl;
				}
			}

			//if packet wasn't received
			if (receivedPacket.getNumber() != receiveCounter) {
				receiveCounter = receivedPacket.getNumber();

				switch (receivedPacket.getType()) {
				case HostCommPacket::ACK: {
					log << Debug << "HostComm: got ACK.";
					//ackSemaphore.unlock();
					ackSemaphore.give();
				}
					break;
				case HostCommPacket::PING:
					log << lock << Debug  << "HostComm: got PING. Sending PONG... " << unlock;
					if (send(pongPacket) == false) {
						log << Debug << "Error" << endl;
					} else {
						log << Debug << "Ok" << endl;
					}
					break;

				case HostCommPacket::PONG:
					log << lock << Debug  << "HostComm: got PONG." << endl << unlock;
					ackSemaphore.give();
					break;
				case HostCommPacket::DEVICE_INFO_REQUEST:
					break;

				default:
					receivedPacket.debug(log);
					incommingPacket.emit(receivedPacket);
				}
			} else {
				log << Debug << "HostComm: discarding packet, was earlier processed." << endl;
			}
		} else {
			log << Debug << "HostComm: packet CRC error." << endl;
		}
	}
}

bool HostComm::readPacket() {
	static size_t dataToRead = 0;

	const size_t bytesAvailable = ioDevice.getAvailableBytes();

	//if receiving new packet
	if (dataToRead == 0) {
		if (bytesAvailable >= sizeof(HostCommPacket::PacketInfo)) {
			ioDevice.read(reinterpret_cast<char*>(&receivedPacket.packetInfo), sizeof(HostCommPacket::PacketInfo));

			dataToRead = receivedPacket.getSize();
			dataToRead -= ioDevice.read(receivedPacket.getDataPtr<char>(), dataToRead);

			//if all packet was received
			if (dataToRead == 0) {
				return true;
			}
		}
	} else {	//else finish receiving packet
		if (bytesAvailable >= dataToRead) {
			char * readPtr = receivedPacket.getDataPtr<char>();
			readPtr += receivedPacket.getSize() - dataToRead;

			dataToRead -= ioDevice.read(readPtr, dataToRead);
			//if all packet was received
			if (dataToRead == 0) {
				return true;
			}
		}
	}
	return false;
}

} // namespace microhal
