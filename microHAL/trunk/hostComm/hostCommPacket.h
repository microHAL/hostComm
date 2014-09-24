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

	HostCommPacket(void * dataPtr) :
			packet(static_cast<Packet *>(dataPtr) ) {

	}

	HostCommPacket(size_t dataSize, uint8_t type, bool needAck,	bool calculateCRC) {
		uint8_t control = 0;

		if (needAck) {
			control = ACK_REQUEST;
		}
		if (calculateCRC) {
			control |= CRC_CALCULATE;
			dataSize += 2; //add 2 bytes for CRC
		}

		packet = static_cast<Packet*>(allocate(dataSize + sizeof(packetInfo)));
		//set up packet
		packet->info.control = control;
		packet->info.type = type;
		packet->info.size = dataSize;
	}

//	~HostCommPacket(){
//
//	}

	uint16_t getSize() const {
		return packet->info.size;
	}

	uint16_t getType() {
		return packet->info.type;
	}

	char *getDataPtr() {
		return &packet->data;
	}
protected:
	typedef struct
		__attribute__((packed)) {
			uint8_t control; //
			uint8_t type; //msb is ack indication
			uint16_t size;
		} packetInfo;

	private:
		struct __attribute__((packed)) Packet {
			packetInfo info;
			char data;
		}*packet;

		bool setNumber(uint_fast8_t number) {
			if (number > 0x0F) {
				return false;
			}
			packet->info.control = (packet->info.control & 0xF0) | number;
			return true;
		}

		uint_fast8_t getNumber() {
			return packet->info.control & 0x0F;
		}

		bool requireACK() {
			return packet->info.control & ACK_REQUEST;
		}

		bool checkCRC() {
			//if packet has crc data
			if (packet->info.control & CRC_CALCULATE) {
				//check crc
				return false;
			}
			return true;
		}

		void calculateCRC() {
			if (packet->info.control & CRC_CALCULATE) {
				//todo calculate CRC
			}
		}

		void* allocate(size_t size){
			static size_t index=0;
			void *ptr = &data[index];
			index += size;
			return ptr;
		}
		static char data[5*1024];
		friend class HostComm;
		friend class HostCommPacket_ACK;
	};

	} /* namespace microhal */

#endif /* HOSTCOMEPACKET_H_ */
