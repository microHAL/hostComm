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

bool HostComm::send(HostCommPacket &packet) {
	std::lock_guard<std::mutex> lock_mutex(sendMutex);
	//count up from 0 to 15
	sentCounter = (sentCounter + 1) & 0x0F;
	//
	packet.setNumber(sentCounter);
	packet.calculateCRC();

	log << lock << INFORMATIONAL << "Sending packet, with type: " << packet.getType() << ", number: " << packet.getNumber() << endl << unlock;
	if(sentPacktToIODevice(packet) == false){
		log << lock << DEBUG << "Unable to sent packet." << endl << unlock;
		return false;
	}

	if (packet.requireACK()) {
		//txPendingPacket = &packet;
		for (uint16_t retransmission = 0; retransmission < maxRetransmissionTry; retransmission++) {
			log << lock << INFORMATIONAL << "Waiting for ACK..." << unlock;
			if (waitForACK(packet)) {
				log << lock << Informational << "ACK OK" << endl << unlock;
				return true;
			}
			log << lock << Informational << "ACK Missing" << endl << "Retransmitting packet." << endl << unlock;
			if(sentPacktToIODevice(packet) == false){
				log << lock << DEBUG << "Unable to sent packet." << endl << unlock;
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
	//std::lock_guard<std::mutex> guard(sendMutex);

	log << lock << DEBUG << "HostComm: Sending PING..." << unlock;
	const bool status = send(pingPacket);
	if (status == true)
		log << lock << Debug << "OK" << endl << unlock;
		if(waitForResponse == true) {
			log << lock << DEBUG  << "HostComm: Waiting for PONG..." << endl << unlock;

			if(ackSemaphore.wait(ackTimeout)) {
				if (receivedPacket.getType() == HostCommPacket::PONG) {
					log << lock << Debug << "OK" << endl << unlock;
					return true;
				} else {
					log << lock << Debug << "ERROR" << endl << unlock;
					return false;
				}
			} else {
				log << lock << INFORMATIONAL  << "Unable to receive PONG, semaphore timeout." << endl << unlock;
				return false;
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
		log << lock << INFORMATIONAL << "Unable to receive ACK, semaphore timeout." << endl << unlock;
	}
	return false;
}

void HostComm::timeProc() {
	if (readPacket() == true) {
		log << lock << DEBUG << "HostComm: got packet." << endl << unlock;
		if (receivedPacket.checkCRC() == true) {
			// if need do send ack
			if (receivedPacket.requireACK() == true) {
				//send ack
				ACKpacket.setPacketToACK(receivedPacket);
				if (send(ACKpacket)) {
					log << lock << DEBUG << "HostComm: ACK sent" << endl << unlock;
				} else {
					log << lock << DEBUG << "HostComm: unable to send ACK."	<< endl << unlock;
				}
			}

			//if packet wasn't received
			if (receivedPacket.getNumber() != receiveCounter) {
				receiveCounter = receivedPacket.getNumber();

				switch (receivedPacket.getType()) {
				case HostCommPacket::ACK: {
					log << lock << DEBUG << "HostComm: got ACK." << unlock;
					ackSemaphore.give();
				}
					break;
				case HostCommPacket::PING:
					log << lock << DEBUG  << "HostComm: got PING. Sending PONG... " << unlock;
					if (send(pongPacket) == false) {
						log << lock << Debug << "Error" << endl << unlock;
					} else {
						log << lock << Debug << "Ok" << endl << unlock;
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
				log << lock << DEBUG << "HostComm: discarding packet, was earlier processed." << endl << unlock;
			}
		} else {
			log << lock << DEBUG << "HostComm: packet CRC error." << endl << unlock;
		}
	}
}

bool HostComm::readPacket() {
	const size_t bytesAvailable = ioDevice.getAvailableBytes();
	if(bytesAvailable) {
		log << lock << DEBUG << "Available bytes: " << bytesAvailable << endl << unlock;
	}

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
