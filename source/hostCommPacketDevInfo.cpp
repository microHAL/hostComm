/*
 * hostCommPacketDevInfo.cpp
 *
 *  Created on: 23 wrz 2015
 *      Author: Pawel
 */

#include "hostCommPacketDevInfo.h"
#include "hostComm.h"

namespace microhal {

DeviceInfoPacket::DeviceInfoPacket(uint16_t manufacturerID, uint16_t productID, uint8_t productVariant, const Version &softwareVersion,
                                   const Version &hardwareVersion)
    : HostCommPacket(this, sizeof(*this), PacketType, true, false),
      m_manufacturerID(manufacturerID),
      m_productID(productID),
      m_hardwareVersion(hardwareVersion),
      m_softwareVersion(softwareVersion),
      m_productVariant(productVariant),
      m_hostCommVersion(HostComm::version) {}

bool DeviceInfoPacket::softwareVersionString(std::string_view versionStr) {
    if (versionStr.size() < m_softwareVersionStr.size()) {
        std::copy(versionStr.begin(), versionStr.end(), m_softwareVersionStr.begin());
        m_softwareVersionStr[versionStr.size()] = 0;
        m_flags |= SoftwareVersionStringPresent;
        return true;
    }
    return false;
}

bool DeviceInfoPacket::deviceName(std::string_view name) {
    if (name.size() < m_deviceName.size()) {
        std::copy(name.begin(), name.end(), m_deviceName.begin());
        m_deviceName[name.size()] = 0;
        m_flags |= DeviceNamePresent;
        return true;
    }
    return false;
}

}  // namespace microhal
