#include "PerformanceController.hpp"
#include "../ACPI/Performance.hpp"
#include "../ACPI/Fans.hpp"
#include "FanController.hpp"
#include <IOKit/IOLib.h>

PerformanceController::PerformanceController(AsusACPI &acpi) : acpi(acpi) {}

IOReturn PerformanceController::setMode(AsusACPIMode mode, uint32_t &firmwareResult)
{
    if (mode != AsusACPIModeBalanced && mode != AsusACPIModePerformance && mode != AsusACPIModeSilent) return kIOReturnBadArgument;
    firmwareResult = 0;
    IOReturn status = AsusACPIPerformance::setMode(acpi, static_cast<uint32_t>(mode), firmwareResult);
    if (status != kIOReturnSuccess) return status;
    return firmwareResult == 1 ? kIOReturnSuccess : kIOReturnError;
}

IOReturn PerformanceController::readMode(AsusACPIMode &mode, uint32_t &rawValue)
{
    uint32_t raw = 0;
    IOReturn status = acpi.get(AsusACPIPerformance::Profile, raw);
    bool vivo = false;
    if (status != kIOReturnSuccess || raw < 0x00010000u || raw - 0x00010000u > AsusACPIModeSilent) {
        status = acpi.get(AsusACPIPerformance::VivoBookProfile, raw);
        if (status != kIOReturnSuccess) return status;
        vivo = true;
    }
    rawValue = raw;
    if (raw < 0x00010000u || raw == UINT32_MAX || raw == UINT32_MAX - 1) return kIOReturnUnsupported;
    uint32_t value = raw - 0x00010000u;
    if (vivo && value == 1) value = 2;
    else if (vivo && value == 2) value = 1;
    if (value > AsusACPIModeSilent) return kIOReturnUnsupported;
    mode = static_cast<AsusACPIMode>(value);
    return kIOReturnSuccess;
}

IOReturn PerformanceController::setStatusMode(AsusACPIMode mode, uint32_t &firmwareResult)
{
    if (mode != AsusACPIModeBalanced && mode != AsusACPIModePerformance && mode != AsusACPIModeSilent) return kIOReturnBadArgument;
    uint8_t payload[] = {0, static_cast<uint8_t>(mode == AsusACPIModeSilent ? 2 : 3)};
    firmwareResult = 0;
    IOReturn status = acpi.setBuffer(AsusACPIPerformance::StatusMode, payload, sizeof(payload), firmwareResult);
    return status != kIOReturnSuccess ? status : firmwareResult == 1 ? kIOReturnSuccess : kIOReturnError;
}

IOReturn PerformanceController::apply(const PerformanceConfiguration &config, bool &usedFanRange)
{
    usedFanRange = false;
    if (config.mode != AsusACPIModeBalanced && config.mode != AsusACPIModePerformance && config.mode != AsusACPIModeSilent) return kIOReturnBadArgument;
    if ((config.cpuCurve == nullptr) != (config.gpuCurve == nullptr)) return kIOReturnBadArgument;
    if (config.middleCurve && !config.cpuCurve) return kIOReturnBadArgument;
    if (config.fanScale > 255 || ((config.hysteresisUp == 0) != (config.hysteresisDown == 0))) return kIOReturnBadArgument;
    if (config.cpuCurve) {
        if (AsusACPIFans::isEmptyCurve(config.cpuCurve) || AsusACPIFans::isEmptyCurve(config.gpuCurve) ||
            (config.middleCurve && AsusACPIFans::isEmptyCurve(config.middleCurve))) return kIOReturnBadArgument;
        for (unsigned i = 8; i < 16; ++i) {
            if (config.cpuCurve[i] > 100 || config.gpuCurve[i] > 100 ||
                (config.middleCurve && config.middleCurve[i] > 100)) return kIOReturnBadArgument;
        }
    }
    if (config.power && !PowerController::isValid(*config.power, config.limits)) return kIOReturnBadArgument;
    uint32_t result = 0;
    IOReturn status;
    if (config.applyStatusMode) {
        status = setStatusMode(config.mode, result);
        if (status != kIOReturnSuccess) return status;
    }
    if (config.manualMode) {
        status = AsusACPIPerformance::setMode(acpi, AsusACPIPerformance::Manual, result);
        if (status == kIOReturnSuccess && result != 1) status = kIOReturnError;
    } else {
        status = setMode(config.mode, result);
    }
    if (status != kIOReturnSuccess) return status;
    // TODO: Windows power plans, GPU clocks and Ryzen SMU settings require macOS equivalents.
    if (config.cpuCurve) {
        IOSleep(100);
        FanController fans(acpi);
        status = fans.applyCurves(config.cpuCurve, config.gpuCurve, config.middleCurve, config.fanScale, config.mode, usedFanRange);
        if (status != kIOReturnSuccess) return status;
        if (config.hysteresisUp && config.hysteresisDown) {
            status = fans.setHysteresis(config.hysteresisUp, config.hysteresisDown, config.middleCurve != nullptr, result);
            if (status != kIOReturnSuccess) return status;
        }
    }
    if (config.power) {
        IOSleep(1000);
        PowerController power(acpi);
        return power.apply(*config.power, config.limits);
    }
    return kIOReturnSuccess;
}
