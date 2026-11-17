#ifndef ASUS_ACPI_KEYBOARD_HPP
#define ASUS_ACPI_KEYBOARD_HPP

#include <stdint.h>
#include "AsusACPI.hpp"

namespace AsusACPIKeyboard {
constexpr uint32_t UniversalControl = 0x00100021;
constexpr uint32_t Brightness = 0x00050021;
constexpr uint32_t BacklightOobe = 0x0005002F;
constexpr uint32_t RGB = 0x00100056;
constexpr uint32_t RGBAlternative = 0x0010005A;
constexpr uint32_t RGBPower = 0x00100057;
constexpr uint32_t FnLock = 0x00100023;
constexpr uint32_t KeyAirplane = 0x88;
constexpr uint32_t KeyLightUp = 0xC4;
constexpr uint32_t KeyLightDown = 0xC5;
constexpr uint32_t KeyBrightnessDown = 0x10;
constexpr uint32_t KeyBrightnessUp = 0x20;
constexpr uint32_t KeySleep = 0x6C;
constexpr uint32_t KeyTouchpad = 0x6B;
constexpr uint32_t KeyMicrophoneMute = 0x7C;
constexpr uint32_t KeyFnLock = 0x4E;
constexpr uint32_t KeyDuoPage = 0x4B;
constexpr uint32_t KeyDuoDisplay = 0x6A;
IOReturn setBrightness(AsusACPI &acpi, uint8_t brightness, uint32_t &result);
IOReturn setRGB(AsusACPI &acpi, uint8_t mode, uint8_t red, uint8_t green, uint8_t blue, uint8_t speed, uint32_t &result);
IOReturn setPower(AsusACPI &acpi, bool awake, bool boot, bool sleep, bool shutdown, uint32_t &result, bool vivoZenPro = false);
}

#endif
