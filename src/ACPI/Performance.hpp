#ifndef ASUS_ACPI_PERFORMANCE_HPP
#define ASUS_ACPI_PERFORMANCE_HPP

#include <stdint.h>
#include "AsusACPI.hpp"

namespace AsusACPIPerformance {
constexpr uint32_t Profile = 0x00120075;
constexpr uint32_t VivoBookProfile = 0x00110019;
constexpr uint32_t StatusMode = 0x00090031;
constexpr uint32_t PowerSavingMode = 0x00090032;
constexpr uint32_t Balanced = 0;
constexpr uint32_t Turbo = 1;
constexpr uint32_t Silent = 2;
constexpr uint32_t FullSpeed = 3;
constexpr uint32_t Manual = 4;
IOReturn setMode(AsusACPI &acpi, uint32_t mode, uint32_t &firmwareResult);
}

#endif
