/*
 * hostComePacket.h
 *
 *  Created on: 13 cze 2014
 *      Author: Dell
 */

#ifndef HOSTCOMEPACKET_H_
#define HOSTCOMEPACKET_H_

#include <stdint.h>
#include <stdio.h>
#include <memory>
#include <type_traits>

#include "diagnostic/diagnostic.h"
#include "CRC/crc32.h"

namespace microhal {

class HostCommPacket {
public:
	enum PacketOptions {
		MAX_PACKET_SIZE = 128
	};
	enum PacketType {
		ACK = 0x00, DEVICE_INFO_REQUEST = 0xFD, PING = 0xFE, PONG = 0xFF
	};
	enum PacketMode {
		NO_ACK = 0x00, NO_CRC = 0x00, ACK_REQUEST = 0x80, CRC_CALCULATE = 0x40
	};

	struct __attribute__((packed)) PacketInfo  {
		uint8_t control; //
		uint8_t type; //msb is ack indication
		uint16_t size;
		uint32_t crc;

		bool operator !=(const PacketInfo &packetInfo) const {
			if(control != packetInfo.control) return true;
			if(type != packetInfo.type) return true;
			if(size != packetInfo.size) return true;
			if(crc != packetInfo.crc) return true;

			return false;
		}

		bool operator ==(const PacketInfo &packetInfo) const {
			if(control != packetInfo.control) return false;
			if(type != packetInfo.type) return false;
			if(size != packetInfo.size) return false;
			if(crc != packetInfo.crc) return false;

			return true;
		}
	};

	constexpr HostCommPacket(HostCommPacket && source) noexcept
		: dataSize(source.dataSize), dataPtr(source.dataPtr), packetInfo(source.packetInfo)	{
	}

	HostCommPacket(uint8_t type, bool needAck) noexcept
		: HostCommPacket(nullptr, 0, type, needAck, false) {
	}

//	~HostCommPacket(){
//
//	}

	uint16_t getSize() const {
		return packetInfo.size;
	}

	uint16_t getType() const {
		return packetInfo.type;
	}

	template<typename T = void>
	T *getDataPtr() const noexcept {
		return static_cast<T*>(dataPtr);
	}

	void debug(diagnostic::Diagnostic &log = diagnostic::diagChannel);
protected:
	HostCommPacket(void* dataPtr, size_t dataSize, uint8_t type = 0xFF, bool needAck = false, bool calculateCRC = false)
			: dataSize(dataSize), dataPtr(dataPtr) {
		uint8_t control = 0;

		if (needAck) {
			control = ACK_REQUEST;
		}
		if (calculateCRC) {
			control |= CRC_CALCULATE;
		}

		//set up packet
		packetInfo.control = control;
		packetInfo.type = type;
		packetInfo.size = dataSize;
	}
private:
	size_t dataSize = 0;
	void *dataPtr = nullptr;
	PacketInfo packetInfo;

	bool setNumber(uint_fast8_t number) {
		if (number > 0x0F) {
			return false;
		}
		packetInfo.control = (packetInfo.control & 0xF0) | number;
		return true;
	}

	uint_fast8_t getNumber() {
		return packetInfo.control & 0x0F;
	}

	bool requireACK() {
		return packetInfo.control & ACK_REQUEST;
	}

	bool checkCRC() {
		//if packet has crc data
		if (packetInfo.control & CRC_CALCULATE) {
			//check crc
			return packetInfo.crc == calculateCRCforAllPacket();
		}
		return true;
	}

	void calculateCRC() {
		if (packetInfo.control & CRC_CALCULATE) {
			packetInfo.crc = calculateCRCforAllPacket();
		}
	}

	uint32_t calculateCRCforPcketInfo() {
		return crc32(&packetInfo, sizeof(packetInfo) - sizeof(packetInfo.crc));
	}

	uint32_t calculateCRCforAllPacket() {
		assert(dataPtr != nullptr);
		assert(packetInfo.size != 0);

		return crc32(dataPtr, packetInfo.size, calculateCRCforPcketInfo());
	}

	friend class HostComm;
	friend class HostCommPacket_ACK;
};

template <typename T, uint8_t packetType, class Allocator = std::allocator<T>>
class HostCommDataPacket : public HostCommPacket {
	static_assert(std::is_trivial<T>::value, "payload (T parameter) must be POD."); //fixme is_pod
//	static_assert(packetType != HostCommPacket::ACK, "These packet type is reserved for ACK packet."); // fixme
	static_assert(packetType != HostCommPacket::DEVICE_INFO_REQUEST, "These packet type is reserved for Device info packet.");
	static_assert(packetType != HostCommPacket::PING, "These packet type is reserved for PING packet.");
	static_assert(packetType != HostCommPacket::PONG, "These packet type is reserved for PONG packet.");
public:
	static constexpr uint8_t PacketType = packetType;

	HostCommDataPacket(bool needAck = false, bool calculateCRC = false)
		: HostCommPacket(allocator.allocate(1), sizeof(T), packetType, needAck, calculateCRC){
	}

	~HostCommDataPacket(){
		allocator.deallocate(payloadPtr(), 1);
	}

	T* payloadPtr() const {
		return HostCommPacket::getDataPtr<T>();
	}

	T& payload() const {
		return *payloadPtr();
	}
private:
	Allocator allocator;
};

} // namespace microhal

#endif // HOSTCOMEPACKET_H_
