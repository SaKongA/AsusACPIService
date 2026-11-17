#include "DisplayController.hpp"

DisplayController::DisplayController(AsusACPI &acpi) : acpi(acpi) {}

IOReturn DisplayController::read(DisplayFeature feature, uint32_t &value)
{
    switch (feature) {
    case DisplayFeature::Overdrive:
    case DisplayFeature::OverdriveSupport:
    case DisplayFeature::MiniLed:
    case DisplayFeature::MiniLedAlternative:
    case DisplayFeature::FullHD:
    case DisplayFeature::HDR:
    case DisplayFeature::OptimalBrightness:
    case DisplayFeature::Initialize:
    case DisplayFeature::ScreenPadToggle:
    case DisplayFeature::ScreenPadBrightness:
        return acpi.getPresentValue(static_cast<uint32_t>(feature), value);
    }
    return kIOReturnBadArgument;
}

IOReturn DisplayController::write(DisplayFeature feature, uint32_t value, uint32_t &firmwareResult)
{
    switch (feature) {
    case DisplayFeature::Overdrive:
    case DisplayFeature::MiniLed:
    case DisplayFeature::MiniLedAlternative:
    case DisplayFeature::FullHD:
    case DisplayFeature::HDR:
    case DisplayFeature::OptimalBrightness:
    case DisplayFeature::Initialize:
    case DisplayFeature::ScreenPadToggle:
    case DisplayFeature::ScreenPadBrightness:
        break;
    default:
        return kIOReturnUnsupported;
    }
    firmwareResult = 0;
    IOReturn status = acpi.set(static_cast<uint32_t>(feature), value, firmwareResult);
    if (status != kIOReturnSuccess) return status;
    return firmwareResult == 1 ? kIOReturnSuccess : kIOReturnError;
}

IOReturn DisplayController::supportsOverdrive(bool &supported)
{
    return AsusACPIDisplay::supportsOverdrive(acpi, supported);
}

IOReturn DisplayController::setMiniLed(uint32_t value, uint32_t &firmwareResult)
{
    bool supported = false;
    IOReturn status = acpi.isSupported(AsusACPIDisplay::MiniLed, supported);
    if (status != kIOReturnSuccess) return status;
    return write(supported ? DisplayFeature::MiniLed : DisplayFeature::MiniLedAlternative, value, firmwareResult);
}

IOReturn DisplayController::setScreenPadBrightness(int32_t percent, uint32_t &firmwareResult)
{
    if (percent < -1 || percent > 100) return kIOReturnBadArgument;
    static const uint8_t brightness[] = {0, 4, 9, 14, 21, 32, 48, 73, 111, 169, 255};
    IOReturn status;
    if (percent >= 0) {
        status = write(DisplayFeature::ScreenPadToggle, 1, firmwareResult);
        if (status != kIOReturnSuccess) return status;
    }
    status = write(DisplayFeature::ScreenPadBrightness, brightness[percent < 0 ? 0 : percent / 10], firmwareResult);
    if (status != kIOReturnSuccess) return status;
    return percent < 0 ? write(DisplayFeature::ScreenPadToggle, 0, firmwareResult) : kIOReturnSuccess;
}

