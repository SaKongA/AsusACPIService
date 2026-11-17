#include "KeyboardController.hpp"

KeyboardController::KeyboardController(AsusACPI &acpi) : acpi(acpi) {}

IOReturn KeyboardController::readFnLock(uint32_t &value)
{
    return acpi.getPresentValue(AsusACPIKeyboard::FnLock, value);
}

IOReturn KeyboardController::setFnLock(bool enabled, uint32_t &firmwareResult)
{
    firmwareResult = 0;
    IOReturn status = acpi.set(AsusACPIKeyboard::FnLock, enabled ? 1 : 0, firmwareResult);
    return status != kIOReturnSuccess ? status : firmwareResult == 1 ? kIOReturnSuccess : kIOReturnError;
}

IOReturn KeyboardController::setBrightness(uint8_t brightness, uint32_t &firmwareResult)
{
    firmwareResult = 0;
    IOReturn status = AsusACPIKeyboard::setBrightness(acpi, brightness, firmwareResult);
    return status != kIOReturnSuccess ? status : firmwareResult == 1 ? kIOReturnSuccess : kIOReturnError;
}

IOReturn KeyboardController::setRGB(uint8_t mode, uint8_t red, uint8_t green, uint8_t blue, uint8_t speed, uint32_t &firmwareResult)
{
    firmwareResult = 0;
    IOReturn status = AsusACPIKeyboard::setRGB(acpi, mode, red, green, blue, speed, firmwareResult);
    return status != kIOReturnSuccess ? status : firmwareResult == 1 ? kIOReturnSuccess : kIOReturnError;
}

IOReturn KeyboardController::setPower(bool awake, bool boot, bool sleep, bool shutdown, bool vivoZenPro, uint32_t &firmwareResult)
{
    firmwareResult = 0;
    IOReturn status = AsusACPIKeyboard::setPower(acpi, awake, boot, sleep, shutdown, firmwareResult, vivoZenPro);
    return status != kIOReturnSuccess ? status : firmwareResult == 1 ? kIOReturnSuccess : kIOReturnError;
}

IOReturn KeyboardController::sendFirmwareKey(uint32_t key, uint32_t &firmwareResult)
{
    if (key != AsusACPIKeyboard::KeyBrightnessUp && key != AsusACPIKeyboard::KeyBrightnessDown && key != AsusACPIKeyboard::KeySleep && key != AsusACPIKeyboard::KeyAirplane) return kIOReturnUnsupported;
    firmwareResult = 0;
    IOReturn status = acpi.set(AsusACPIKeyboard::UniversalControl, key, firmwareResult);
    return status != kIOReturnSuccess ? status : firmwareResult == 1 ? kIOReturnSuccess : kIOReturnError;
}
