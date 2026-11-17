#include "TouchpadController.hpp"
#include "../../Diagnostics/KernelLog.hpp"
#include <IOKit/IOMessage.h>
#include <IOKit/IOService.h>
#include <libkern/c++/OSIterator.h>

namespace {
constexpr UInt32 SetTouchStatus = iokit_vendor_specific_msg(100);
constexpr UInt32 GetTouchStatus = iokit_vendor_specific_msg(101);
}

TouchpadController::TouchpadController(IOService *sender) : sender(sender) {}

IOReturn TouchpadController::findDevice(IOService *&device)
{
    device = nullptr;
    OSDictionary *matching = IOService::serviceMatching("VoodooI2CMultitouchHIDEventDriver");
    if (!matching) return kIOReturnNoMemory;
    OSIterator *iterator = IOService::getMatchingServices(matching);
    if (!iterator) return kIOReturnNotFound;
    IOReturn status = kIOReturnNotFound;
    while (OSObject *object = iterator->getNextObject()) {
        IOService *candidate = OSDynamicCast(IOService, object);
        if (!candidate || candidate->isInactive()) continue;
        if (device) { device->release(); device = nullptr; status = kIOReturnExclusiveAccess; break; }
        candidate->retain();
        device = candidate;
        status = kIOReturnSuccess;
    }
    iterator->release();
    return status;
}

IOReturn TouchpadController::read(bool &enabled)
{
    IOService *device = nullptr;
    IOReturn status = findDevice(device);
    if (status != kIOReturnSuccess) return status;
    bool value = false;
    status = device->message(GetTouchStatus, sender, &value);
    device->release();
    if (status == kIOReturnSuccess) enabled = value;
    else KernelLog::error("Touchpad", "read", "status=0x%08x", static_cast<unsigned>(status));
    return status;
}

IOReturn TouchpadController::set(bool enabled)
{
    IOService *device = nullptr;
    IOReturn status = findDevice(device);
    if (status != kIOReturnSuccess) return status;
    bool requested = enabled;
    status = device->message(SetTouchStatus, sender, &requested);
    device->release();
    if (status == kIOReturnSuccess) KernelLog::info("Touchpad", "set", "enabled=%u", enabled ? 1u : 0u);
    else KernelLog::error("Touchpad", "set", "status=0x%08x", static_cast<unsigned>(status));
    return status;
}
