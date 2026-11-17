#include "DeviceController.hpp"

DeviceController::DeviceController(AsusACPI &acpi) : acpi(acpi) {}

IOReturn DeviceController::read(DeviceFeature feature, uint32_t &value)
{
    switch (feature) {
    case DeviceFeature::MicrophoneMuteLed:
    case DeviceFeature::SoundMuteLed:
    case DeviceFeature::SlateMode:
    case DeviceFeature::TabletState:
    case DeviceFeature::TentState:
    case DeviceFeature::CameraShutter:
    case DeviceFeature::CameraLed:
    case DeviceFeature::StatusLed:
    case DeviceFeature::BootSound:
    case DeviceFeature::CpuCores:
    case DeviceFeature::CpuCoresMax:
    case DeviceFeature::CpuCoresMin:
        return acpi.getPresentValue(static_cast<uint32_t>(feature), value);
    }
    return kIOReturnBadArgument;
}

IOReturn DeviceController::write(DeviceFeature feature, uint32_t value, uint32_t &firmwareResult)
{
    switch (feature) {
    case DeviceFeature::MicrophoneMuteLed:
    case DeviceFeature::SoundMuteLed:
    case DeviceFeature::CameraShutter:
    case DeviceFeature::StatusLed:
    case DeviceFeature::BootSound:
    case DeviceFeature::CpuCores:
        break;
    default:
        return kIOReturnUnsupported;
    }
    firmwareResult = 0;
    IOReturn status = acpi.set(static_cast<uint32_t>(feature), value, firmwareResult);
    if (status != kIOReturnSuccess) return status;
    return firmwareResult == 1 ? kIOReturnSuccess : kIOReturnError;
}

IOReturn DeviceController::readCores(DeviceFeature feature, uint8_t &efficiency, uint8_t &performance)
{
    return AsusACPIDevices::readCores(acpi, static_cast<uint32_t>(feature), efficiency, performance);
}

IOReturn DeviceController::setCores(uint8_t efficiency, uint8_t performance, uint32_t &firmwareResult)
{
    firmwareResult = 0;
    IOReturn status = AsusACPIDevices::setCores(acpi, efficiency, performance, firmwareResult);
    return status != kIOReturnSuccess ? status : firmwareResult == 1 ? kIOReturnSuccess : kIOReturnError;
}

IOReturn DeviceController::toggleCameraShutter(bool &cameraEnabled, uint32_t &firmwareResult)
{
    uint32_t value = 0;
    IOReturn status = read(DeviceFeature::CameraShutter, value);
    if (status != kIOReturnSuccess) return status;
    uint32_t state = value & 1u;
    switch (value & ~1u) {
    case 0:
        status = write(DeviceFeature::CameraShutter, state ^ 1u, firmwareResult);
        cameraEnabled = state != 0;
        return status;
    case 0x40000:
        cameraEnabled = state != 0;
        firmwareResult = 0;
        return kIOReturnSuccess;
    case 0x100000:
        status = write(DeviceFeature::CameraShutter, 4u | state, firmwareResult);
        cameraEnabled = state == 0;
        return status;
    default:
        // TODO: Windows AsusHotkey camera control requires a macOS equivalent.
        return kIOReturnUnsupported;
    }
}

IOReturn DeviceController::setStatusLed(bool enabled, uint32_t &firmwareResult)
{
    return write(DeviceFeature::StatusLed, enabled ? 7 : 0, firmwareResult);
}

