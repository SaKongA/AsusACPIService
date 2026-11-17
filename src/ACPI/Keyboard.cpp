#include "Keyboard.hpp"

IOReturn AsusACPIKeyboard::setBrightness(AsusACPI &acpi, uint8_t brightness, uint32_t &result)
{
    if (brightness > 0x7F) return kIOReturnBadArgument;
    return acpi.set(Brightness, 0x80u | brightness, result);
}

IOReturn AsusACPIKeyboard::setRGB(AsusACPI &acpi, uint8_t mode, uint8_t red, uint8_t green, uint8_t blue, uint8_t speed, uint32_t &result)
{
    uint8_t payload[] = {0xB4, mode, red, green, blue, speed};
    IOReturn status = acpi.setBuffer(RGB, payload, sizeof(payload), result);
    if (status != kIOReturnSuccess || result == 1) return status;
    payload[0] = 0xB3;
    status = acpi.setBuffer(RGBAlternative, payload, sizeof(payload), result);
    if (status != kIOReturnSuccess) return status;
    payload[0] = 0xB4;
    return acpi.setBuffer(RGBAlternative, payload, sizeof(payload), result);
}

IOReturn AsusACPIKeyboard::setPower(AsusACPI &acpi, bool awake, bool boot, bool sleep, bool shutdown, uint32_t &result, bool vivoZenPro)
{
    uint32_t state = 0x01BD;
    if (boot) state |= 0x03u << 16;
    if (awake) state |= 0x0Cu << 16;
    if (sleep) state |= 0x30u << 16;
    if (shutdown) state |= 0xC0u << 16;
    IOReturn status = acpi.set(RGBPower, state, result);
    if (status != kIOReturnSuccess || !vivoZenPro) return status;
    bool supported = false;
    status = acpi.isSupported(BacklightOobe, supported);
    if (status != kIOReturnSuccess || !supported) return status;
    uint32_t ignored = 0;
    return acpi.set(BacklightOobe, 1, ignored);
}
