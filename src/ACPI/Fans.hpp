#ifndef ASUS_ACPI_FANS_HPP
#define ASUS_ACPI_FANS_HPP

#include <stdint.h>
#include "AsusACPI.hpp"

namespace AsusACPIFans {
constexpr uint32_t CPUSpeed = 0x00110013;
constexpr uint32_t GPUSpeed = 0x00110014;
constexpr uint32_t MiddleSpeed = 0x00110031;
constexpr uint32_t CPURange = 0x00110022;
constexpr uint32_t GPURange = 0x00110023;
constexpr uint32_t CPUCurve = 0x00110024;
constexpr uint32_t GPUCurve = 0x00110025;
constexpr uint32_t MiddleCurve = 0x00110032;
constexpr uint32_t CPUFloor = 0x00110026;
constexpr uint32_t GPUFloor = 0x00110027;
constexpr uint32_t MiddleFloor = 0x00110033;
constexpr uint32_t Hysteresis = 0x00110034;
constexpr uint32_t CPUTemperature = 0x00120094;
constexpr uint32_t GPUTemperature = 0x00120097;

IOReturn readSpeed(AsusACPI &acpi, uint32_t device, uint32_t &rpm);
IOReturn readCPUSpeed(AsusACPI &acpi, uint32_t &rpm);
IOReturn readGPUSpeed(AsusACPI &acpi, uint32_t &rpm);
IOReturn readMiddleSpeed(AsusACPI &acpi, uint32_t &rpm);
IOReturn readCurve(AsusACPI &acpi, uint32_t device, uint32_t mode, void *output, size_t &outputSize);
IOReturn readFloor(AsusACPI &acpi, uint32_t device, void *output, size_t &outputSize);
IOReturn readHysteresis(AsusACPI &acpi, uint8_t &up, uint8_t &down);
IOReturn writeHysteresis(AsusACPI &acpi, uint8_t up, uint8_t down, bool middleFan, uint32_t &result);
IOReturn writeCurve(AsusACPI &acpi, uint32_t device, const uint8_t curve[16], uint32_t &result, uint32_t scale = 100);
IOReturn writeRange(AsusACPI &acpi, uint32_t device, const uint8_t curve[16], uint32_t &result);
bool isEmptyCurve(const uint8_t curve[16]);
IOReturn fixCurve(uint8_t curve[16], bool clampPoints);
}

#endif
