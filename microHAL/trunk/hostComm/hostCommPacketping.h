/*
 * hostCommPacketping.h
 *
 *  Created on: 13 cze 2014
 *      Author: Dell
 */

#ifndef HOSTCOMMPACKETPING_H_
#define HOSTCOMMPACKETPING_H_

#include <stdint.h>

#include "hostCommPacket.h"
#include "../diagnostic/diagnostic.h"

namespace microhal {

class HostCommPacket_ACK: public HostCommPacket {
public:
//	static HostCommPacket_ACK & getInstance() {
//		static HostCommPacket_ACK packet;
//
//		return packet;
//	}

	void setPacketToACK(HostCommPacket &packet){
		HostCommPacket::packetInfo *ACKinfo = reinterpret_cast<HostCommPacket::packetInfo*>(&this->packet->data);
		*ACKinfo = packet.packet->info;
	}

	bool isAcknowledged(HostCommPacket &packet){
		HostCommPacket::packetInfo *ACKinfo = reinterpret_cast<HostCommPacket::packetInfo*>(&this->packet->data);
		if(packet.packet->info.control != ACKinfo->control) return false;
		if(packet.packet->info.type != ACKinfo->type) return false;
		if(packet.packet->info.size !=ACKinfo->size) return false;

		return true;
	}

	void log(){
		HostCommPacket::packetInfo *ACKinfo = reinterpret_cast<HostCommPacket::packetInfo*>(&this->packet->data);
		diagnostic::diagChannel << diagnostic::Debug << "control: " << ACKinfo->control << diagnostic::endl
								<< "type: " << ACKinfo->type << diagnostic::endl
								<< "size: " << ACKinfo->size << diagnostic::endl;
	}

	HostCommPacket_ACK() :
			HostCommPacket(sizeof(HostCommPacket::packetInfo), HostCommPacket::ACK, false, false) {
	}
	~HostCommPacket_ACK() {

	}
};

} /* namespace microhal */

#endif /* HOSTCOMMPACKETPING_H_ */
