#ifndef ASUS_ACPI_POWER_HPP
#define ASUS_ACPI_POWER_HPP

#include <stdint.h>
#include "AsusACPI.hpp"

namespace AsusACPIPower {
constexpr uint32_t ChargerMode = 0x0012006C;
constexpr uint32_t BatteryDischarge = 0x0012005A;
constexpr uint32_t BatteryLimit = 0x00120057;
constexpr uint32_t ApuSlowBoost = 0x001200A0;
constexpr uint32_t CPUEdc = 0x001200A1;
constexpr uint32_t CPUTdc = 0x001200A2;
constexpr uint32_t ApuSustained = 0x001200A3;
constexpr uint32_t CPUPpt = 0x001200B0;
constexpr uint32_t TotalPpt = 0x001200B1;
constexpr uint32_t CrossLoadLegacy = 0x0012009A;
constexpr uint32_t GPUToCPU = 0x0012009C;
constexpr uint32_t CPUTemperatureLimit = 0x0012009E;
constexpr uint32_t CrossLoad = 0x0012009F;
constexpr uint32_t GPUBoost = 0x001200C0;
constexpr uint32_t ApuFastBoost = 0x001200C1;
constexpr uint32_t GPUTemperatureTarget = 0x001200C2;
constexpr uint32_t GPUBase = 0x00120099;
constexpr uint32_t GPUAdditional = 0x00120098;
IOReturn readBatteryDischarge(AsusACPI &acpi, int16_t &centiWatts);
}

#endif
