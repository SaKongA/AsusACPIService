#include "Power.hpp"

IOReturn AsusACPIPower::readBatteryDischarge(AsusACPI &acpi, int16_t &centiWatts)
{
    uint8_t data[16] = {};
    size_t size = sizeof(data);
    IOReturn status = acpi.getBuffer(BatteryDischarge, 0, data, size);
    if (status != kIOReturnSuccess) return status;
    if (size < 3) return kIOReturnBadMedia;
    if (data[2] == 0) return kIOReturnUnsupported;
    centiWatts = static_cast<int16_t>(static_cast<uint16_t>(data[0]) | (static_cast<uint16_t>(data[1]) << 8));
    return kIOReturnSuccess;
}
