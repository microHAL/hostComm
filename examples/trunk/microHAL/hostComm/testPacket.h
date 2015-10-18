/*
 * testPacket.h
 *
 *  Created on: 24 cze 2015
 *      Author: Pawel
 */

#ifndef TESTPACKET_H_
#define TESTPACKET_H_

#include "diagnostic/diagnostic.h"
#include "hostComm/hostCommPacket.h"

namespace microhal {

class testData{
public:
	void log(diagnostic::Diagnostic &log = diagnostic::diagChannel);

	void setCounter(uint8_t cnt) { counter = cnt; }

	uint8_t getCounter() { return counter; }
private:
	uint8_t counter;
};


class testPacket : public HostCommDataPacket<testData, 0x21>{
public:
	testPacket() : HostCommDataPacket<testData, 0x21>::HostCommDataPacket(false, false) {
	}
	~testPacket(){
	}

	enum {
		Request = 0x20
	};
};

} // namespace microhal

#endif // TESTPACKET_H_
