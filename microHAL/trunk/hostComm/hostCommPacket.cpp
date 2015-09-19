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
	log << lock << DEBUG << endl
		<< "\tpacket type: " << packetInfo.type << endl
		<< "\tdata size: " << packetInfo.size << endl
		<< "\trequire ACK: " << requireACK() << endl
		<< "\tdata ptr: " << toHex(reinterpret_cast<uint64_t>(dataPtr)) << endl;
	if(packetInfo.size) {
		log << Debug << "\tPacket data: " << toHex(getDataPtr<uint8_t>(), packetInfo.size) << endl;
	}
	log << unlock;
}

} /* namespace microhal */
