#ifndef DEVICE_CONTROLLER_HPP
#define DEVICE_CONTROLLER_HPP

#include "../ACPI/Devices.hpp"

enum class DeviceFeature : uint32_t {
    MicrophoneMuteLed = AsusACPIDevices::MicrophoneMuteLed,
    SoundMuteLed = AsusACPIDevices::SoundMuteLed,
    SlateMode = AsusACPIDevices::SlateMode,
    TabletState = AsusACPIDevices::TabletState,
    TentState = AsusACPIDevices::TentState,
    CameraShutter = AsusACPIDevices::CameraShutter,
    CameraLed = AsusACPIDevices::CameraLed,
    StatusLed = AsusACPIDevices::StatusLed,
    BootSound = AsusACPIDevices::BootSound,
    CpuCores = AsusACPIDevices::CPUCores,
    CpuCoresMax = AsusACPIDevices::CPUCoresMax,
    CpuCoresMin = AsusACPIDevices::CPUCoresMin
};

class DeviceController {
public:
    explicit DeviceController(AsusACPI &acpi);
    IOReturn read(DeviceFeature feature, uint32_t &value);
    IOReturn write(DeviceFeature feature, uint32_t value, uint32_t &firmwareResult);
    IOReturn readCores(DeviceFeature feature, uint8_t &efficiency, uint8_t &performance);
    IOReturn setCores(uint8_t efficiency, uint8_t performance, uint32_t &firmwareResult);
    IOReturn toggleCameraShutter(bool &cameraEnabled, uint32_t &firmwareResult);
    IOReturn setStatusLed(bool enabled, uint32_t &firmwareResult);

private:
    AsusACPI &acpi;
};

#endif
