#include "Display.hpp"

IOReturn AsusACPIDisplay::supportsOverdrive(AsusACPI &acpi, bool &supported)
{
    uint32_t value = 0;
    IOReturn status = acpi.getPresentValue(OverdriveSupport, value);
    if (status != kIOReturnSuccess) return status;
    supported = value == 1;
    return kIOReturnSuccess;
}
