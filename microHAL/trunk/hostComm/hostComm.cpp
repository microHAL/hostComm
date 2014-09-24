/*
 * hostComm.cpp
 *
 *  Created on: 13 cze 2014
 *      Author: Dell
 */

#include "hostComm.h"

namespace microhal {
//HostCommPacket(0, HostCommPacket::PONG, false, false)

HostCommPacket HostComm::pingPacket(0, HostCommPacket::PING, false, false);
HostCommPacket HostComm::pongPacket(0, HostCommPacket::PONG, false, false);



void HostComm::timeProc() {
	if (readPacket() == true) {
		diagChannel << Debug << "HostComm: got packet." << endl;
		if (receivedPacket.checkCRC() == true) {
			// if need do send ack
			if (receivedPacket.requireACK() == true) {
				//send ack
				HostCommPacket_ACK::getInstance().setPacketToACK(
						receivedPacket);
				if (send(HostCommPacket_ACK::getInstance())) {
					diagChannel << Debug << "HostComm: ACK sent" << endl;
				} else {
					diagChannel << Debug << "HostComm: unable to send ACK."
							<< endl;
				}
			}

			//if packet wasn't received
			if (receivedPacket.getNumber() != receiveCounter) {
				receiveCounter = receivedPacket.getNumber();

				switch (receivedPacket.getType()) {
				case HostCommPacket::ACK: {
					diagChannel << Debug << "HostComm: got ACK.";
					HostCommPacket_ACK &ack =
							static_cast<HostCommPacket_ACK&>(receivedPacket);
					if (ack.isAcknowledged(*txPendingPacket)) {
						txPendingPacket = nullptr;
					}
				}
					break;
				case HostCommPacket::PING:
					diagChannel << Debug
							<< "HostComm: got PING. Sending PONG... ";
					if (send(pongPacket) == false) {
						diagChannel << Debug << "Error" << endl;
					} else {
						diagChannel << Debug << "Ok" << endl;
					}
					break;

				case HostCommPacket::PONG:
					diagChannel << Debug << "HostComm: got PONG." << endl;

					break;
				case HostCommPacket::DEVICE_INFO_REQUEST:
					break;

				default:
					//diagChannel << Debug << endl;
					//diagChannel.write((uint8_t*)&receivedPacket.packet.info.control,16,16);
					diagChannel << Debug << "HostComm: emit signal" << endl;
					incommingPacket.emit(receivedPacket);
				}
			} else {
				diagChannel << Debug
						<< "HostComm: discarding packet, was earlier processed."
						<< endl;
			}
		} else {
			diagChannel << Debug << "HostComm: packet CRC error." << endl;
		}
	}
}

bool HostComm::readPacket() {
	static size_t dataToRead = 0;
	char buffer[200];

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

} /* namespace microhal */
