#ifndef ASUS_ACPI_DEVICES_HPP
#define ASUS_ACPI_DEVICES_HPP

#include <stdint.h>
#include "AsusACPI.hpp"

namespace AsusACPIDevices {
constexpr uint32_t MicrophoneMuteLed = 0x00040017;
constexpr uint32_t SoundMuteLed = 0x0004001C;
constexpr uint32_t SlateMode = 0x00120063;
constexpr uint32_t TabletState = 0x00060077;
constexpr uint32_t TentState = 0x00060062;
constexpr uint32_t CameraShutter = 0x00060078;
constexpr uint32_t CameraLed = 0x00060079;
constexpr uint32_t StatusLed = 0x000600C2;
constexpr uint32_t BootSound = 0x00130022;
constexpr uint32_t CPUCores = 0x001200D2;
constexpr uint32_t CPUCoresMax = 0x001200D3;
constexpr uint32_t CPUCoresMin = 0x001200D4;
IOReturn readCores(AsusACPI &acpi, uint32_t device, uint8_t &efficiency, uint8_t &performance);
IOReturn setCores(AsusACPI &acpi, uint8_t efficiency, uint8_t performance, uint32_t &result);
}

#endif
