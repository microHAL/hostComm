/*
 * testPacket.cpp
 *
 *  Created on: 24 cze 2015
 *      Author: Pawel
 */

#include "testPacket.h"

namespace microhal {
using namespace diagnostic;

void testData::log(diagnostic::Diagnostic &log){
	log << lock << Debug <<  "counter value: " << counter << endl << unlock;
}

} // namespace microhal
