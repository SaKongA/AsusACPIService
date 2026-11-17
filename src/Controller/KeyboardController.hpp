#ifndef KEYBOARD_CONTROLLER_HPP
#define KEYBOARD_CONTROLLER_HPP

#include "../ACPI/Keyboard.hpp"

class KeyboardController {
public:
    explicit KeyboardController(AsusACPI &acpi);
    IOReturn readFnLock(uint32_t &value);
    IOReturn setFnLock(bool enabled, uint32_t &firmwareResult);
    IOReturn setBrightness(uint8_t brightness, uint32_t &firmwareResult);
    IOReturn setRGB(uint8_t mode, uint8_t red, uint8_t green, uint8_t blue, uint8_t speed, uint32_t &firmwareResult);
    IOReturn setPower(bool awake, bool boot, bool sleep, bool shutdown, bool vivoZenPro, uint32_t &firmwareResult);
    IOReturn sendFirmwareKey(uint32_t key, uint32_t &firmwareResult);

private:
    AsusACPI &acpi;
};

#endif
