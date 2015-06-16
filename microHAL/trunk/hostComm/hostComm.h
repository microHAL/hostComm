/*
 * hostComm.h
 *
 *  Created on: 13 cze 2014
 *      Author: Dell
 */

#ifndef HOSTCOMM_H_
#define HOSTCOMM_H_

#include <stdint.h>
#include "signalSlot/signalSlot.h"
#include "hostCommPacket.h"
#include "hostCommPacketping.h"
#include "IODevice.h"
#include "../diagnostic/diagnostic.h"
#include <mutex>
#include "semaphore.h"


namespace microhal {

class HostComm {
public:
	HostComm(IODevice &ioDevice, diagnostic::Diagnostic &log = diagnostic::diagChannel) :
			ioDevice(ioDevice), log(log), receivedPacket(packetBuffer) {
	}

	bool send(HostCommPacket &packet);

	void timeProc();

	template<typename _Rep, typename _Period>
	void setACKtimeout(const std::chrono::duration<_Rep, _Period>& timeout) {
		ackTimeout = timeout;
	}

	void setMaxRetransmissionCount(uint8_t retransmission) {
		maxRetransmissionTry = retransmission;
	}


	bool ping(bool waitForResponse);
	bool isAvailablePacket();

	bool getPendingPacket(HostCommPacket *packet) {
		packet = &receivedPacket;
	}

	Signal<HostCommPacket &> incommingPacket;
private:
	semaphore ackSemaphore;

	std::mutex sendMutex;
	std::chrono::milliseconds ackTimeout = {std::chrono::milliseconds{1000}};
	uint8_t sentCounter = 0; //this counter contain last number of sent frame. This counter is increased when sending new frame but now when retransmitting frame.
	uint8_t receiveCounter = 0; //this counter contain last number of received frame, is used to detect receive of retransmitted frame.
	uint8_t maxRetransmissionTry = 3;
	IODevice &ioDevice;
	diagnostic::Diagnostic &log;

	struct {
		uint32_t sentPacketCounter = 0;
		uint32_t receivedPacketCounter = 0;
	} statistics;

	uint8_t packetBuffer[200];
	HostCommPacket receivedPacket;
	HostCommPacket *txPendingPacket = nullptr;

	//static packets
	static HostCommPacket_ACK ACKpacket;
	static HostCommPacket pingPacket;
	static HostCommPacket pongPacket;

	bool sentPacktToIODevice(HostCommPacket &packet);
	bool waitForACK(HostCommPacket &packetToACK);

	bool readPacket();
};

} // namespace microhal

#endif /* HOSTCOMM_H_ */
