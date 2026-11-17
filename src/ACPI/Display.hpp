#ifndef ASUS_ACPI_DISPLAY_HPP
#define ASUS_ACPI_DISPLAY_HPP

#include <stdint.h>
#include "AsusACPI.hpp"

namespace AsusACPIDisplay {
constexpr uint32_t Overdrive = 0x00050019;
constexpr uint32_t OverdriveSupport = 0x00050020;
constexpr uint32_t MiniLed = 0x0005001E;
constexpr uint32_t MiniLedAlternative = 0x0005002E;
constexpr uint32_t FullHD = 0x0005001C;
constexpr uint32_t HDR = 0x00050071;
constexpr uint32_t OptimalBrightness = 0x0005002A;
constexpr uint32_t Initialize = 0x00050011;
constexpr uint32_t ScreenPadToggle = 0x00050031;
constexpr uint32_t ScreenPadBrightness = 0x00050032;
IOReturn supportsOverdrive(AsusACPI &acpi, bool &supported);
}

#endif
