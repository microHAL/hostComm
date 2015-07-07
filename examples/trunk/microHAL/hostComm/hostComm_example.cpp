/*
 * hostComm_example.h
 *
 *  Created on: 20 gru 2014
 *      Author: Dell
 */

#include "microhal.h"
#include "hostComm/hostComm.h"
#include "microhal_bsp.h"

#include "allocators/stackAllocator.h"

#include <vector>

#include "testPacket.h"
#include "RawMeasurement.h"

using namespace microhal;
using namespace diagnostic;

using namespace std::literals::chrono_literals;

HostComm hostComm(communicationPort);

volatile bool getTestRequest = false;

void proceedPacket(HostCommPacket &packet) {
	if (packet.getType() == testPacket::TestPacketRequest) {
		diagChannel << Debug << "Got testPacket request. Sending testPacket ...";

		getTestRequest = true;
	}

	if (packet.getType() == testPacket::PacketType) {
		diagChannel << Debug << "Got testPacket.";
		testPacket &packetC = static_cast<testPacket&>(packet);

		packetC.payload().log();
	}
}

int main(){
//	debugPort.clear();
//
//	debugPort.setDataBits(SerialPort::Data8);
//	debugPort.setStopBits(SerialPort::OneStop);
//	debugPort.setParity(SerialPort::NoParity);
//	debugPort.setBaudRate(SerialPort::Baud115200, SerialPort::AllDirections);
	debugPort.open(SerialPort::ReadWrite);

	communicationPort.setDataBits(SerialPort::Data8);
	communicationPort.setStopBits(SerialPort::OneStop);
	communicationPort.setParity(SerialPort::NoParity);
	communicationPort.setBaudRate(SerialPort::Baud115200, SerialPort::AllDirections);
	communicationPort.open(SerialPort::ReadWrite);

	debugPort.write("\n\r------------------- HostComm example -------------------------\n\r");

	diagChannel.setOutputDevice(debugPort);

	//connect function that will be called when new packet will be received
	hostComm.incommingPacket.connect(proceedPacket);

	//create and run hostComm proc task
	std::thread hostCommThread([](){
		while(1){
			std::this_thread::sleep_for(1ms);
			hostComm.timeProc();
		}
	});



//	std::vector<int, stackAllocator<int, 11> > test;
//
//	for(size_t i=0; i<12; i++){
//		diagChannel << Debug << (uint32_t)i << endl;
//		test.push_back(i);
//	}

	testPacket packet;

	HostCommPacket testRequest(testPacket::TestPacketRequest, false, false);
	//hostComm.send(testRequest);

	RawMeasurement measurement;

	for(size_t i = 0; i< 256; i++){
		measurement.payload()[i].photodiode1 = (uint8_t)i;
		measurement.payload()[i].photodiode2 = (uint8_t)i;
		measurement.payload()[i].refCurrent = 0;
	}
	measurement.debug();

	diagChannel << Debug << "starting main loop." << endl;
	while(1){
		std::this_thread::sleep_for(5s);
		//hostComm.ping(true);

		hostComm.send(measurement);

		if(getTestRequest){
			getTestRequest = false;
			diagChannel << Debug << "Sending test packet." << endl;
			packet.payload().setCounter(5);
			hostComm.send(packet);
		}
	}

	return 0;
}
