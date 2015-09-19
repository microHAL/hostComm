/*
 * hostCommPacketping.h
 *
 *  Created on: 13 cze 2014
 *      Author: Dell
 */

#ifndef HOSTCOMMPACKETACK_H_
#define HOSTCOMMPACKETACK_H_

#include <stdint.h>

#include "hostCommPacket.h"
#include "../diagnostic/diagnostic.h"

namespace microhal {

class HostCommPacket_ACK : public HostCommDataPacket < HostCommPacket::PacketInfo, HostCommPacket::ACK> {
public:
	void setPacketToACK(HostCommPacket &packet) noexcept {
		payload() = packet.packetInfo;
	}

	bool isAcknowledged(HostCommPacket &packet) const noexcept {
		if(packet.packetInfo != payload()) return false;

		return true;
	}

	void log(diagnostic::Diagnostic &log = diagnostic::diagChannel) const noexcept {
		const HostCommPacket::PacketInfo *ACKinfo = payloadPtr();
		log << diagnostic::Debug << "control: " << ACKinfo->control << diagnostic::endl
								<< "type: " << ACKinfo->type << diagnostic::endl
								<< "size: " << ACKinfo->size << diagnostic::endl;
	}
};

} // namespace microhal

#endif // HOSTCOMMPACKETACK_H_
