/*
 * hostCommPacketDevInfo.h
 *
 *  Created on: 23 wrz 2015
 *      Author: Pawel
 */

#ifndef HOSTCOMMPACKETDEVINFO_H_
#define HOSTCOMMPACKETDEVINFO_H_

#include <optional>
#include "hostCommPacket.h"

namespace microhal {

struct Version {
    uint8_t major;
    uint8_t minor;
    uint16_t patch;
};

constexpr bool operator==(const Version &lhs, const Version &rhs) {
    if (lhs.major != rhs.major) return false;
    if (lhs.minor != rhs.minor) return false;
    if (lhs.patch != rhs.patch) return false;
    return true;
}

class DeviceInfoPacket : public HostCommPacket {
 public:
    template <typename Type>
    using optional = std::optional<Type>;
    enum : uint8_t { UniqueIDPresent = 0b0001, SerialNumberPresent = 0b0010, SoftwareVersionStringPresent = 0b0100, DeviceNamePresent = 0b1000 };

    static constexpr const uint8_t PacketType = HostCommPacket::DEVICE_INFO;
    static constexpr const uint8_t Request = HostCommPacket::DEVICE_INFO_REQUEST;

    DeviceInfoPacket(uint16_t manufacturerID, uint16_t productID, uint8_t productVariant, const Version &softwareVersion,
                     const Version &hardwareVersion);

    uint16_t manufacturerId() const { return m_manufacturerID; }
    uint16_t productId() const { return m_productID; }
    uint8_t productVariant() const { return m_productVariant; }

    Version softwareVersion() const { return m_softwareVersion; }
    void softwareVersion(Version version) { m_softwareVersion = version; }

    Version hardwareVersion() const { return m_hardwareVersion; }
    void hardwareVersion(Version version) { m_hardwareVersion = version; }

    void uniqueId(uint64_t id) {
        m_uniqueId = id;
        m_flags |= UniqueIDPresent;
    }
    optional<uint64_t> uniqueId() const {
        if (m_flags & UniqueIDPresent) return m_uniqueId;
        return std::nullopt;
    }

    void serialNumber(uint32_t serial) {
        m_serialNumber = serial;
        m_flags |= SerialNumberPresent;
    }
    optional<uint32_t> serialNumber() const {
        if (m_flags & SerialNumberPresent) return m_serialNumber;
        return std::nullopt;
    }

    std::string_view softwareVersionString() {
        if (m_flags & SoftwareVersionStringPresent) return {m_softwareVersionStr.data()};
        return {};
    }
    bool softwareVersionString(std::string_view versionStr);

    std::string_view deviceName() const {
        if (m_flags & DeviceNamePresent) return {m_deviceName.data()};
        return {};
    }
    bool deviceName(std::string_view name);

    uint8_t hostCommVersion() const { return m_hostCommVersion; }
    uint16_t maxPacketDataSize() const { return m_maxPacketDataSize; }

 private:
    uint64_t m_uniqueId = 0;
    uint32_t m_serialNumber = 0;
    uint16_t m_manufacturerID;
    uint16_t m_productID;
    const uint16_t m_maxPacketDataSize = HostCommPacket::maxPacketDataSize;
    Version m_hardwareVersion;
    Version m_softwareVersion;
    uint8_t m_productVariant;
    const uint8_t m_hostCommVersion;
    uint8_t m_flags = 0;
    std::array<char, 20> m_softwareVersionStr{};
    std::array<char, 20> m_deviceName{};
};

static_assert(sizeof(DeviceInfoPacket) <= HostCommPacket::maxPacketDataSize,
              "HostComm max packet size has to be greater than size of DeviceInfoPacket");

} /* namespace microhal */

#endif /* HOSTCOMMPACKETDEVINFO_H_ */
