#ifndef POWER_CONTROLLER_HPP
#define POWER_CONTROLLER_HPP

#include "../ACPI/Power.hpp"
#include "../ACPI/ModelLimits.hpp"

struct PowerConfiguration {
    int32_t total = -1;
    int32_t cpu = -1;
    int32_t slow = -1;
    int32_t fast = -1;
    int32_t gpuToCpu = -1;
    int32_t crossLoad = -1;
    int32_t cpuTemperature = -1;
    int32_t gpuBoost = -1;
    int32_t gpuTemperature = -1;
    int32_t gpuAdditional = -1;
    bool amdCPU = false;
    bool integratedOnly = false;
};

enum class PowerFeature : uint32_t {
    Charger = AsusACPIPower::ChargerMode,
    BatteryLimit = AsusACPIPower::BatteryLimit,
    ApuSlowBoost = AsusACPIPower::ApuSlowBoost,
    CpuEdc = AsusACPIPower::CPUEdc,
    CpuTdc = AsusACPIPower::CPUTdc,
    ApuSustained = AsusACPIPower::ApuSustained,
    CpuPpt = AsusACPIPower::CPUPpt,
    TotalPpt = AsusACPIPower::TotalPpt,
    CrossLoadLegacy = AsusACPIPower::CrossLoadLegacy,
    GpuToCpu = AsusACPIPower::GPUToCPU,
    CpuTemperatureLimit = AsusACPIPower::CPUTemperatureLimit,
    CrossLoad = AsusACPIPower::CrossLoad,
    GpuBoost = AsusACPIPower::GPUBoost,
    ApuFastBoost = AsusACPIPower::ApuFastBoost,
    GpuTemperatureTarget = AsusACPIPower::GPUTemperatureTarget,
    GpuBase = AsusACPIPower::GPUBase,
    GpuAdditional = AsusACPIPower::GPUAdditional
};

class PowerController {
public:
    explicit PowerController(AsusACPI &acpi);
    IOReturn read(PowerFeature feature, uint32_t &value);
    IOReturn write(PowerFeature feature, uint32_t value, uint32_t &firmwareResult);
    IOReturn readBatteryDischarge(int16_t &centiWatts);
    static bool isValid(const PowerConfiguration &configuration, const AsusACPIModelLimits &limits);
    IOReturn apply(const PowerConfiguration &configuration, const AsusACPIModelLimits &limits);

private:
    AsusACPI &acpi;
    IOReturn writeIfSupported(PowerFeature feature, uint32_t value, bool &written);
};

#endif
