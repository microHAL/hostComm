/*
 * hostComm.h
 *
 *  Created on: 13 cze 2014
 *      Author: Dell
 */

#ifndef HOSTCOMM_H_
#define HOSTCOMM_H_

#include <stdint.h>
#include "signalSlot.h"
#include "hostCommPacket.h"
#include "hostCommPacketping.h"
#include "IODevice.h"
#include "../diagnostic/diagnostic.h"

namespace microhal {

class HostComm {
public:
	HostComm(IODevice &ioDevice) :
			ioDevice(ioDevice), receivedPacket(packetBuffer) {

	}

	void timeProc();

	void enable() {

	}

	void disable() {

	}

	bool waitForACK(HostCommPacket &packetToACK) {
		volatile uint32_t timeout = 300000;
		while (txPendingPacket != nullptr) {
			timeProc();
			if (timeout-- == 0) {
				return false;
			}
		}
		return true;
	}

	void setMaxRetransmissionCount(uint8_t retransmissionCount) {

	}

	bool send(HostCommPacket &packet) {
		//count up from 0 to 15
		sentCounter = (sentCounter + 1) & 0x0F;
		//
		packet.setNumber(sentCounter);
		packet.calculateCRC();

		ioDevice.write((char*)packet.packet, packet.getSize() + sizeof(HostCommPacket::packetInfo));

		if (packet.requireACK()) {
			txPendingPacket = &packet;
			for (uint16_t retransmission = 0; retransmission < 3;
					retransmission++) {
				if (waitForACK(packet)) {
					return true;
				}
				ioDevice.write((char*)packet.packet, packet.getSize() + sizeof(HostCommPacket::packetInfo));
			}
			//unable to deliver packet
			return false;
		}
		return true;
	}

	bool ping(bool waitForResponse) {
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

	bool isAvailablePacket();

	bool getPendingPacket(HostCommPacket *packet) {
		packet = &receivedPacket;
	}

	Signal<HostCommPacket &> incommingPacket;
private:
	uint8_t sentCounter = 0;
	uint8_t receiveCounter = 0;
	IODevice &ioDevice;

	uint8_t packetBuffer[200];
	HostCommPacket receivedPacket;
	HostCommPacket *txPendingPacket = nullptr;

	//static packets
	static HostCommPacket pingPacket;
	static HostCommPacket pongPacket;

	bool readPacket();
};

} /* namespace microhal */

#endif /* HOSTCOMM_H_ */
