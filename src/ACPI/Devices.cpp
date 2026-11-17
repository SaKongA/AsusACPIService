#include "Devices.hpp"

IOReturn AsusACPIDevices::readCores(AsusACPI &acpi, uint32_t device, uint8_t &efficiency, uint8_t &performance)
{
    if (device != CPUCores && device != CPUCoresMax && device != CPUCoresMin) return kIOReturnBadArgument;
    uint32_t value = 0;
    IOReturn status = acpi.getPresentValue(device, value);
    if (status != kIOReturnSuccess) return status;
    efficiency = static_cast<uint8_t>(value >> 8);
    performance = static_cast<uint8_t>(value);
    return kIOReturnSuccess;
}

IOReturn AsusACPIDevices::setCores(AsusACPI &acpi, uint8_t efficiency, uint8_t performance, uint32_t &result)
{
    if (efficiency > 16 || performance < 1 || performance > 16) return kIOReturnBadArgument;
    return acpi.set(CPUCores, (static_cast<uint32_t>(efficiency) << 8) | performance, result);
}
