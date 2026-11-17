#ifndef PERFORMANCE_CONTROLLER_HPP
#define PERFORMANCE_CONTROLLER_HPP

#include "../ACPI/AsusACPI.hpp"
#include "../../include/AsusACPIProtocol.h"
#include "PowerController.hpp"

struct PerformanceConfiguration {
    AsusACPIMode mode = AsusACPIModeBalanced;
    bool applyStatusMode = false;
    bool manualMode = false;
    const uint8_t *cpuCurve = nullptr;
    const uint8_t *gpuCurve = nullptr;
    const uint8_t *middleCurve = nullptr;
    uint32_t fanScale = 100;
    uint8_t hysteresisUp = 0;
    uint8_t hysteresisDown = 0;
    const PowerConfiguration *power = nullptr;
    AsusACPIModelLimits limits = {};
};

class PerformanceController {
public:
    explicit PerformanceController(AsusACPI &acpi);
    IOReturn setMode(AsusACPIMode mode, uint32_t &firmwareResult);
    IOReturn readMode(AsusACPIMode &mode, uint32_t &rawValue);
    IOReturn setStatusMode(AsusACPIMode mode, uint32_t &firmwareResult);
    IOReturn apply(const PerformanceConfiguration &configuration, bool &usedFanRange);

private:
    AsusACPI &acpi;
};

#endif
