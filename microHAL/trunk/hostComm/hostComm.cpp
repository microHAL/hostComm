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

HostCommPacket HostComm::pingPacket(0, HostCommPacket::PING, false, false);
HostCommPacket HostComm::pongPacket(0, HostCommPacket::PONG, false, false);
HostCommPacket_ACK HostComm::ACKpacket;

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
	if(reinterpret_cast<char*>(packet.packet + sizeof(HostCommPacket::packetInfo)) == packet.getDataPtr()){
		const size_t sizeToTransfer = packet.getSize() + sizeof(HostCommPacket::packetInfo);
		if(ioDevice.write((char*)packet.packet, sizeToTransfer) == sizeToTransfer) return true;
	} else {
		if(ioDevice.write((char*)packet.packet, sizeof(HostCommPacket::packetInfo)) !=  sizeof(HostCommPacket::packetInfo)) return false;
		if(ioDevice.write((char*)packet.getDataPtr(), packet.getSize()) == packet.getSize()) return true;
	}
	return false;
}

bool HostComm::ping(bool waitForResponse) {
	bool status = send(pingPacket);
	if (waitForResponse == true) {
		volatile uint32_t timeout = 300000;
		while (txPendingPacket != nullptr) {
			timeProc();
			if (timeout-- == 0) {
				status = false;
			}
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
					log << Debug
							<< "HostComm: got PING. Sending PONG... ";
					if (send(pongPacket) == false) {
						log << Debug << "Error" << endl;
					} else {
						log << Debug << "Ok" << endl;
					}
					break;

				case HostCommPacket::PONG:
					log << Debug << "HostComm: got PONG." << endl;

					break;
				case HostCommPacket::DEVICE_INFO_REQUEST:
					break;

				default:
					log << Debug << "HostComm: emit signal" << endl;
					incommingPacket.emit(receivedPacket);
				}
			} else {
				log << Debug
						<< "HostComm: discarding packet, was earlier processed."
						<< endl;
			}
		} else {
			log << Debug << "HostComm: packet CRC error." << endl;
		}
	}
}

bool HostComm::readPacket() {
	static size_t dataToRead = 0;

	size_t bytesAvailable = ioDevice.getAvailableBytes();

	//if receiving new packet
	if (dataToRead == 0) {
		if (bytesAvailable >= sizeof(HostCommPacket::packetInfo)) {
			ioDevice.read((char *) receivedPacket.packet, sizeof(HostCommPacket::packetInfo));

			dataToRead = receivedPacket.getSize();
			dataToRead -= ioDevice.read(receivedPacket.getDataPtr(), dataToRead);

			//if all packet was received
			if (dataToRead == 0) {
				return true;
			}
		}
	} else {	//else finish receiving packet
		if (bytesAvailable >= dataToRead) {
			char * readPtr = receivedPacket.getDataPtr();
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
