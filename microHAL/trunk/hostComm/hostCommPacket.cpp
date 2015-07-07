/*
 * hostComePacket.cpp
 *
 *  Created on: 13 cze 2014
 *      Author: Dell
 */

#include "hostCommPacket.h"

namespace microhal {

using namespace diagnostic;

void HostCommPacket::debug(diagnostic::Diagnostic &log){
	log << lock << Debug
		<< "packet type: " << packetInfo.type << endl
		<< "data size: " << packetInfo.size << endl
		<< "require ACK: " << requireACK() << endl
		<< "data ptr: " << toHex(reinterpret_cast<uint64_t>(dataPtr)) << endl;
	if(packetInfo.size) {
		log << Debug << "Packet data: " << toHex(getDataPtr<uint8_t>(), packetInfo.size) << endl;
	}
	log << unlock;
}

} /* namespace microhal */
