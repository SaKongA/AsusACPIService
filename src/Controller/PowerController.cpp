#include "PowerController.hpp"
#include "../ACPI/Graphics.hpp"

PowerController::PowerController(AsusACPI &acpi) : acpi(acpi) {}

IOReturn PowerController::read(PowerFeature feature, uint32_t &value)
{
    switch (feature) {
    case PowerFeature::Charger:
    case PowerFeature::BatteryLimit:
    case PowerFeature::ApuSlowBoost:
    case PowerFeature::CpuEdc:
    case PowerFeature::CpuTdc:
    case PowerFeature::ApuSustained:
    case PowerFeature::CpuPpt:
    case PowerFeature::TotalPpt:
    case PowerFeature::CrossLoadLegacy:
    case PowerFeature::GpuToCpu:
    case PowerFeature::CpuTemperatureLimit:
    case PowerFeature::CrossLoad:
    case PowerFeature::GpuBoost:
    case PowerFeature::ApuFastBoost:
    case PowerFeature::GpuTemperatureTarget:
    case PowerFeature::GpuBase:
    case PowerFeature::GpuAdditional:
        return acpi.getPresentValue(static_cast<uint32_t>(feature), value);
    }
    return kIOReturnBadArgument;
}

IOReturn PowerController::write(PowerFeature feature, uint32_t value, uint32_t &firmwareResult)
{
    switch (feature) {
    case PowerFeature::BatteryLimit:
    case PowerFeature::ApuSlowBoost:
    case PowerFeature::CpuEdc:
    case PowerFeature::CpuTdc:
    case PowerFeature::ApuSustained:
    case PowerFeature::CpuPpt:
    case PowerFeature::TotalPpt:
    case PowerFeature::CrossLoadLegacy:
    case PowerFeature::GpuToCpu:
    case PowerFeature::CpuTemperatureLimit:
    case PowerFeature::CrossLoad:
    case PowerFeature::GpuBoost:
    case PowerFeature::ApuFastBoost:
    case PowerFeature::GpuTemperatureTarget:
    case PowerFeature::GpuAdditional:
        break;
    default:
        return kIOReturnUnsupported;
    }
    firmwareResult = 0;
    IOReturn status = acpi.set(static_cast<uint32_t>(feature), value, firmwareResult);
    if (status != kIOReturnSuccess) return status;
    return firmwareResult == 1 ? kIOReturnSuccess : kIOReturnError;
}

IOReturn PowerController::readBatteryDischarge(int16_t &centiWatts)
{
    return AsusACPIPower::readBatteryDischarge(acpi, centiWatts);
}

IOReturn PowerController::writeIfSupported(PowerFeature feature, uint32_t value, bool &written)
{
    bool supported = false;
    IOReturn status = acpi.isSupported(static_cast<uint32_t>(feature), supported);
    if (status != kIOReturnSuccess || !supported) return status;
    uint32_t result = 0;
    status = write(feature, value, result);
    if (status == kIOReturnSuccess) written = true;
    return status;
}

bool PowerController::isValid(const PowerConfiguration &config, const AsusACPIModelLimits &limits)
{
    auto valid = [](int32_t value, uint32_t minimum, uint32_t maximum) {
        return value == -1 || (value >= 0 && static_cast<uint32_t>(value) >= minimum && static_cast<uint32_t>(value) <= maximum);
    };
    return !(!valid(config.total, 5, limits.maxTotal) || !valid(config.cpu, 5, limits.maxCPU) ||
        !valid(config.slow, 5, limits.maxTotal) || !valid(config.fast, 5, limits.maxTotal) ||
        !valid(config.gpuToCpu, 0, 10) || !valid(config.crossLoad, 0, limits.maxCrossLoad) ||
        !valid(config.cpuTemperature, 75, limits.maxCPUTemperature) || !valid(config.gpuBoost, 5, limits.maxGPUBoost) ||
        !valid(config.gpuTemperature, 75, 87) || !valid(config.gpuAdditional, 0, 75));
}

IOReturn PowerController::apply(const PowerConfiguration &config, const AsusACPIModelLimits &limits)
{
    if (!isValid(config, limits)) return kIOReturnBadArgument;

    bool allAmd = false;
    IOReturn status = AsusACPIGraphics::isAllAmdPpt(acpi, config.integratedOnly, allAmd);
    if (status != kIOReturnSuccess) return status;
    bool written = false;
    auto applyValue = [&](PowerFeature feature, int32_t value) -> IOReturn {
        return value < 0 ? kIOReturnSuccess : writeIfSupported(feature, static_cast<uint32_t>(value), written);
    };

    int32_t slow = allAmd || config.slow < 0 ? config.total : config.slow;
    if ((status = applyValue(PowerFeature::ApuSustained, config.total)) != kIOReturnSuccess) return status;
    if ((status = applyValue(PowerFeature::ApuSlowBoost, slow)) != kIOReturnSuccess) return status;
    if (allAmd && (status = applyValue(PowerFeature::CpuPpt, config.cpu)) != kIOReturnSuccess) return status;
    if (config.amdCPU && !allAmd && (status = applyValue(PowerFeature::ApuFastBoost, config.fast)) != kIOReturnSuccess) return status;
    if ((status = applyValue(PowerFeature::GpuToCpu, config.gpuToCpu)) != kIOReturnSuccess) return status;
    if ((status = applyValue(PowerFeature::CrossLoad, config.crossLoad)) != kIOReturnSuccess) return status;
    if ((status = applyValue(PowerFeature::CrossLoadLegacy, config.crossLoad)) != kIOReturnSuccess) return status;
    if ((status = applyValue(PowerFeature::CpuTemperatureLimit, config.cpuTemperature)) != kIOReturnSuccess) return status;
    if ((status = applyValue(PowerFeature::GpuAdditional, config.gpuAdditional)) != kIOReturnSuccess) return status;
    if ((status = applyValue(PowerFeature::GpuBoost, config.gpuBoost)) != kIOReturnSuccess) return status;
    if ((status = applyValue(PowerFeature::GpuTemperatureTarget, config.gpuTemperature)) != kIOReturnSuccess) return status;
    // TODO: Windows Ryzen SMU fallback requires a macOS hardware-control implementation.
    return written ? kIOReturnSuccess : kIOReturnUnsupported;
}

