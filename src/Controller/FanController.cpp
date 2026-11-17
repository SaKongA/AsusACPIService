#include "FanController.hpp"
#include "../ACPI/Fans.hpp"
#include "PerformanceController.hpp"

FanController::FanController(AsusACPI &acpi) : acpi(acpi) {}

IOReturn FanController::readSpeed(AsusFan fan, uint32_t &rpm)
{
    switch (fan) {
    case AsusFan::CPU: return AsusACPIFans::readCPUSpeed(acpi, rpm);
    case AsusFan::GPU: return AsusACPIFans::readGPUSpeed(acpi, rpm);
    case AsusFan::Middle: return AsusACPIFans::readMiddleSpeed(acpi, rpm);
    }
    return kIOReturnBadArgument;
}

IOReturn FanController::readTemperature(AsusFan fan, uint32_t &celsius)
{
    uint32_t device = 0;
    switch (fan) {
    case AsusFan::CPU: device = AsusACPIFans::CPUTemperature; break;
    case AsusFan::GPU: device = AsusACPIFans::GPUTemperature; break;
    case AsusFan::Middle: return kIOReturnUnsupported;
    default: return kIOReturnBadArgument;
    }

    uint32_t value = 0;
    IOReturn status = acpi.getPresentValue(device, value);
    if (status != kIOReturnSuccess) return status;
    if (value == 0 || value >= 125) return kIOReturnBadMedia;
    celsius = value;
    return kIOReturnSuccess;
}

IOReturn FanController::readCurve(AsusFan fan, uint32_t mode, void *output, size_t &outputSize)
{
    switch (fan) {
    case AsusFan::CPU: return AsusACPIFans::readCurve(acpi, AsusACPIFans::CPUCurve, mode, output, outputSize);
    case AsusFan::GPU: return AsusACPIFans::readCurve(acpi, AsusACPIFans::GPUCurve, mode, output, outputSize);
    case AsusFan::Middle: return AsusACPIFans::readCurve(acpi, AsusACPIFans::MiddleCurve, mode, output, outputSize);
    }
    return kIOReturnBadArgument;
}

IOReturn FanController::readFloor(AsusFan fan, void *output, size_t &outputSize)
{
    switch (fan) {
    case AsusFan::CPU: return AsusACPIFans::readFloor(acpi, AsusACPIFans::CPUFloor, output, outputSize);
    case AsusFan::GPU: return AsusACPIFans::readFloor(acpi, AsusACPIFans::GPUFloor, output, outputSize);
    case AsusFan::Middle: return AsusACPIFans::readFloor(acpi, AsusACPIFans::MiddleFloor, output, outputSize);
    }
    return kIOReturnBadArgument;
}

IOReturn FanController::readHysteresis(uint8_t &up, uint8_t &down)
{
    return AsusACPIFans::readHysteresis(acpi, up, down);
}

IOReturn FanController::setHysteresis(uint8_t up, uint8_t down, bool middleFan, uint32_t &firmwareResult)
{
    firmwareResult = 0;
    IOReturn status = AsusACPIFans::writeHysteresis(acpi, up, down, middleFan, firmwareResult);
    return status != kIOReturnSuccess ? status : firmwareResult == 1 ? kIOReturnSuccess : kIOReturnError;
}

IOReturn FanController::setCurve(AsusFan fan, const uint8_t curve[16], uint32_t scale, uint32_t &firmwareResult)
{
    uint32_t device;
    switch (fan) {
    case AsusFan::CPU: device = AsusACPIFans::CPUCurve; break;
    case AsusFan::GPU: device = AsusACPIFans::GPUCurve; break;
    case AsusFan::Middle: device = AsusACPIFans::MiddleCurve; break;
    default: return kIOReturnBadArgument;
    }
    firmwareResult = 0;
    IOReturn status = AsusACPIFans::writeCurve(acpi, device, curve, firmwareResult, scale);
    return status != kIOReturnSuccess ? status : firmwareResult == 1 ? kIOReturnSuccess : kIOReturnError;
}

IOReturn FanController::setRange(AsusFan fan, const uint8_t curve[16], uint32_t &firmwareResult)
{
    uint32_t device;
    switch (fan) {
    case AsusFan::CPU: device = AsusACPIFans::CPURange; break;
    case AsusFan::GPU: device = AsusACPIFans::GPURange; break;
    case AsusFan::Middle: return kIOReturnUnsupported;
    default: return kIOReturnBadArgument;
    }
    firmwareResult = 0;
    IOReturn status = AsusACPIFans::writeRange(acpi, device, curve, firmwareResult);
    return status != kIOReturnSuccess ? status : firmwareResult == 1 ? kIOReturnSuccess : kIOReturnError;
}

IOReturn FanController::fixCurve(uint8_t curve[16], bool clampPoints)
{
    return AsusACPIFans::fixCurve(curve, clampPoints);
}

IOReturn FanController::applyCurves(const uint8_t cpu[16], const uint8_t gpu[16], const uint8_t *middle, uint32_t scale, AsusACPIMode fallbackMode, bool &usedRange)
{
    if (!cpu || !gpu || AsusACPIFans::isEmptyCurve(cpu) || AsusACPIFans::isEmptyCurve(gpu) || scale > 255) return kIOReturnBadArgument;
    if (middle && AsusACPIFans::isEmptyCurve(middle)) return kIOReturnBadArgument;
    for (unsigned i = 8; i < 16; ++i) {
        if (cpu[i] > 100 || gpu[i] > 100 || (middle && middle[i] > 100)) return kIOReturnBadArgument;
    }
    if (fallbackMode != AsusACPIModeBalanced && fallbackMode != AsusACPIModePerformance && fallbackMode != AsusACPIModeSilent) return kIOReturnBadArgument;
    usedRange = false;
    uint32_t result = 0;
    IOReturn cpuStatus = setCurve(AsusFan::CPU, cpu, scale, result);
    IOReturn gpuStatus = setCurve(AsusFan::GPU, gpu, scale, result);
    if (cpuStatus != kIOReturnSuccess || gpuStatus != kIOReturnSuccess) {
        cpuStatus = setRange(AsusFan::CPU, cpu, result);
        gpuStatus = setRange(AsusFan::GPU, gpu, result);
        if (cpuStatus != kIOReturnSuccess || gpuStatus != kIOReturnSuccess) {
            PerformanceController performance(acpi);
            IOReturn reset = performance.setMode(fallbackMode, result);
            if (reset != kIOReturnSuccess) return reset;
            return cpuStatus != kIOReturnSuccess ? cpuStatus : gpuStatus;
        }
        usedRange = true;
    }
    if (middle) return setCurve(AsusFan::Middle, middle, scale, result);
    return kIOReturnSuccess;
}
