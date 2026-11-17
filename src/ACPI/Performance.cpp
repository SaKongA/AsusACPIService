#include "Performance.hpp"

IOReturn AsusACPIPerformance::setMode(AsusACPI &acpi, uint32_t mode, uint32_t &firmwareResult)
{
    if (mode > Manual) return kIOReturnBadArgument;
    bool supported = false;
    IOReturn status = acpi.isSupported(Profile, supported);
    if (status != kIOReturnSuccess) return status;
    if (supported) return acpi.set(Profile, mode, firmwareResult);

    status = acpi.isSupported(VivoBookProfile, supported);
    if (status != kIOReturnSuccess) return status;
    uint32_t mapped = mode == Turbo ? Silent : mode == Silent ? Turbo : mode;
    if (supported) return acpi.set(VivoBookProfile, mapped, firmwareResult);
    status = acpi.set(Profile, mode, firmwareResult);
    if (status == kIOReturnSuccess && firmwareResult == 1) return status;
    return acpi.set(VivoBookProfile, mapped, firmwareResult);
}
