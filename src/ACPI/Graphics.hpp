#ifndef ASUS_ACPI_GRAPHICS_HPP
#define ASUS_ACPI_GRAPHICS_HPP

#include <stdint.h>
#include "AsusACPI.hpp"

namespace AsusACPIGraphics {
constexpr uint32_t RogEco = 0x00090020;
constexpr uint32_t VivoEco = 0x00090120;
constexpr uint32_t ExternalGPUConnected = 0x00090018;
constexpr uint32_t ExternalGPU = 0x00090019;
constexpr uint32_t ExternalDisplay = 0x00090030;
constexpr uint32_t RogMux = 0x00090016;
constexpr uint32_t VivoMux = 0x00090026;
constexpr uint32_t ApuMemory = 0x000600C1;
constexpr uint32_t VramMemory = 0x000600C4;
IOReturn setEco(AsusACPI &acpi, uint32_t device, bool enabled, uint32_t &result);
IOReturn externalDisplayConnected(AsusACPI &acpi, bool &connected);
IOReturn externalGPUConnected(AsusACPI &acpi, bool &connected);
IOReturn setApuMemory(AsusACPI &acpi, uint32_t index, uint32_t &result);
IOReturn readApuMemory(AsusACPI &acpi, uint32_t &index);
IOReturn readVramOptions(AsusACPI &acpi, uint16_t *options, size_t &count, uint32_t &unitMB, uint32_t &current);
IOReturn readVramMemory(AsusACPI &acpi, uint32_t &current);
IOReturn setVramMemory(AsusACPI &acpi, uint32_t value, uint32_t &result);
IOReturn isAllAmdPpt(AsusACPI &acpi, bool integratedOnly, bool &supported);
IOReturn isNvidiaGPU(AsusACPI &acpi, uint32_t ecoDevice, bool integratedOnly, bool ally, bool &supported);
}

#endif
