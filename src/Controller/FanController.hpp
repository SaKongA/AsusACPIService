#ifndef FAN_CONTROLLER_HPP
#define FAN_CONTROLLER_HPP

#include "../ACPI/AsusACPI.hpp"
#include "../../include/AsusACPIProtocol.h"

enum class AsusFan {
    CPU,
    GPU,
    Middle
};

class FanController {
public:
    explicit FanController(AsusACPI &acpi);
    IOReturn readSpeed(AsusFan fan, uint32_t &rpm);
    IOReturn readTemperature(AsusFan fan, uint32_t &celsius);
    IOReturn readCurve(AsusFan fan, uint32_t mode, void *output, size_t &outputSize);
    IOReturn readFloor(AsusFan fan, void *output, size_t &outputSize);
    IOReturn readHysteresis(uint8_t &up, uint8_t &down);
    IOReturn setHysteresis(uint8_t up, uint8_t down, bool middleFan, uint32_t &firmwareResult);
    IOReturn setCurve(AsusFan fan, const uint8_t curve[16], uint32_t scale, uint32_t &firmwareResult);
    IOReturn setRange(AsusFan fan, const uint8_t curve[16], uint32_t &firmwareResult);
    IOReturn fixCurve(uint8_t curve[16], bool clampPoints);
    IOReturn applyCurves(const uint8_t cpu[16], const uint8_t gpu[16], const uint8_t *middle, uint32_t scale, AsusACPIMode fallbackMode, bool &usedRange);

private:
    AsusACPI &acpi;
};

#endif
