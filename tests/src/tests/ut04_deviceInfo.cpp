/*
 * ut04_deviceInfo.cpp
 *
 *  Created on: May 20, 2021
 *      Author: pokas
 */

#ifndef SRC_TESTS_UT04_DEVICEINFO_CPP_
#define SRC_TESTS_UT04_DEVICEINFO_CPP_

#include "bsp.h"
#include "doctest.h"
#include "hostComm.h"
#include "hostCommPacketDevInfo.h"

using namespace microhal;
using namespace std::chrono_literals;

static HostCommPacket *received = nullptr;

static void proceedPacket(HostCommPacket &packet) {
    received = &packet;
}

TEST_CASE("Test Device info packet") {
    DeviceInfoPacket deviceInfo(0xFFAA, 0x55, 1, {1, 0, 0}, {0, 1, 0});

    CHECK(deviceInfo.manufacturerId() == 0xFFAA);
    CHECK(deviceInfo.productId() == 0x55);
    CHECK(deviceInfo.productVariant() == 1);
    CHECK(deviceInfo.softwareVersion() == Version{1, 0, 0});
    CHECK(deviceInfo.hardwareVersion() == Version{0, 1, 0});
    CHECK(deviceInfo.hostCommVersion() == HostComm::version);
    CHECK(deviceInfo.maxPacketDataSize() == HostCommPacket::maxPacketDataSize);
    CHECK(deviceInfo.uniqueId().has_value() == false);
    CHECK(deviceInfo.serialNumber().has_value() == false);
    CHECK(deviceInfo.softwareVersionString() == std::string_view{});
    CHECK(deviceInfo.deviceName() == std::string_view{});

    deviceInfo.uniqueId(0x1234567890);
    deviceInfo.serialNumber(0xAABBCCDD);
    CHECK_FALSE(deviceInfo.softwareVersionString("12345678901234567890"));
    CHECK_FALSE(deviceInfo.deviceName("12345678901234567890"));
    CHECK(deviceInfo.manufacturerId() == 0xFFAA);
    CHECK(deviceInfo.productId() == 0x55);
    CHECK(deviceInfo.productVariant() == 1);
    CHECK(deviceInfo.softwareVersion() == Version{1, 0, 0});
    CHECK(deviceInfo.hardwareVersion() == Version{0, 1, 0});
    CHECK(deviceInfo.hostCommVersion() == HostComm::version);
    CHECK(deviceInfo.maxPacketDataSize() == HostCommPacket::maxPacketDataSize);
    CHECK(deviceInfo.uniqueId().has_value() == true);
    CHECK(deviceInfo.uniqueId().value() == 0x1234567890);
    CHECK(deviceInfo.serialNumber().has_value() == true);
    CHECK(deviceInfo.serialNumber().value() == 0xAABBCCDD);
    CHECK(deviceInfo.softwareVersionString() == std::string_view{});
    CHECK(deviceInfo.deviceName() == std::string_view{});

    CHECK(deviceInfo.softwareVersionString("software version"));
    CHECK(deviceInfo.softwareVersionString() == std::string_view{"software version"});

    CHECK(deviceInfo.deviceName("Device Name String"));
    CHECK(deviceInfo.deviceName() == std::string_view{"Device Name String"});
}

TEST_CASE("Packet sending") {
    // ports should be open
    REQUIRE(communicationPortA.isOpen());
    REQUIRE(communicationPortB.isOpen());

    // clear ports
    REQUIRE(communicationPortA.clear());
    REQUIRE(communicationPortB.clear());

    HostComm hostCommA(communicationPortA, debugPort, "HostComm A: ");
    HostComm hostCommB(communicationPortB, debugPort, "HostComm B: ");

    INFO("Starting timeProc thread.");
    // create and run hostComm proc task
    hostCommA.startHostCommThread();
    hostCommB.startHostCommThread();

    INFO("Connecting incoming packet slot.");
    // connect function that will be called when new packet will be received
    hostCommB.incommingPacket.connect(proceedPacket);

    DeviceInfoPacket deviceInfo(0xFFAA, 0x55, 1, {1, 0, 0}, {0, 1, 0});
    CHECK(hostCommA.registerDeviceInfoPacket(&deviceInfo));

    CHECK(hostCommB.requestDeviceInfo());
    std::this_thread::sleep_for(100ms);
    REQUIRE(received != nullptr);
    CHECK(received->getType() == deviceInfo.getType());
    CHECK(received->getSize() == deviceInfo.getSize());
    CHECK(deviceInfo.manufacturerId() == 0xFFAA);
    CHECK(deviceInfo.productId() == 0x55);
    CHECK(deviceInfo.productVariant() == 1);
    CHECK(deviceInfo.softwareVersion() == Version{1, 0, 0});
    CHECK(deviceInfo.hardwareVersion() == Version{0, 1, 0});
    CHECK(deviceInfo.hostCommVersion() == HostComm::version);
    CHECK(deviceInfo.maxPacketDataSize() == HostCommPacket::maxPacketDataSize);
}

#endif /* SRC_TESTS_UT04_DEVICEINFO_CPP_ */
