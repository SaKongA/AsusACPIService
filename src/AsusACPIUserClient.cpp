#include "AsusACPIUserClient.hpp"
#include "AsusACPIService.hpp"
#include "ACPI/Performance.hpp"
#include "Controller/FanController.hpp"
#include "Controller/PerformanceController.hpp"
#include "Controller/PowerController.hpp"
#include "Controller/GraphicsController.hpp"
#include "Controller/DisplayController.hpp"
#include "Controller/KeyboardController.hpp"
#include "Controller/DeviceController.hpp"
#include "Controller/Forwarding/TouchpadController.hpp"
#include "Diagnostics/KernelLog.hpp"
#include <libkern/libkern.h>

#define super IOUserClient
OSDefineMetaClassAndStructors(AsusACPIUserClient, IOUserClient)

AsusACPIUserClient *AsusACPIUserClient::create()
{
    return new AsusACPIUserClient;
}

bool AsusACPIUserClient::initWithTask(task_t owningTask, void *securityID, UInt32 type, OSDictionary *properties)
{
    if (type != 0 || !super::initWithTask(owningTask, securityID, type, properties)) return false;
    return true;
}

bool AsusACPIUserClient::start(IOService *provider)
{
    service = OSDynamicCast(AsusACPIService, provider);
    if (!service || !super::start(provider)) { service = nullptr; return false; }
    return true;
}

void AsusACPIUserClient::stop(IOService *provider)
{
    KernelLog::info("UserClient", "stop", "user client stopped");
    service = nullptr;
    super::stop(provider);
}

IOReturn AsusACPIUserClient::clientClose()
{
    terminate();
    return kIOReturnSuccess;
}

IOReturn AsusACPIUserClient::clientDied()
{
    return clientClose();
}

IOReturn AsusACPIUserClient::externalMethod(uint32_t selector, IOExternalMethodArguments *arguments, IOExternalMethodDispatch *, OSObject *, void *)
{
    static IOExternalMethodDispatch methods[] = {
        {getState, 0, 0, 0, sizeof(AsusACPIState)},
        {setMode, 1, 0, 0, 0},
        {controller, 0, sizeof(AsusACPIRequest), 0, sizeof(AsusACPIResponse)}
    };
    if (selector >= sizeof(methods) / sizeof(methods[0]) || !arguments) {
        KernelLog::warning("UserClient", "dispatch", "unsupported selector=%u", static_cast<unsigned>(selector));
        return kIOReturnUnsupported;
    }
    IOReturn status = super::externalMethod(selector, arguments, &methods[selector], this, nullptr);
    if (status != kIOReturnSuccess && status != kIOReturnUnsupported) {
        KernelLog::error("UserClient", "dispatch", "selector=%u status=0x%08x", static_cast<unsigned>(selector), static_cast<unsigned>(status));
    }
    return status;
}

IOReturn AsusACPIUserClient::controller(OSObject *target, void *, IOExternalMethodArguments *arguments)
{
    auto *client = OSDynamicCast(AsusACPIUserClient, target);
    if (!client || !client->service) return kIOReturnNotAttached;
    if (!arguments->structureInput || !arguments->structureOutput) return kIOReturnBadArgument;
    const auto &request = *static_cast<const AsusACPIRequest *>(arguments->structureInput);
    return client->dispatchController(request, *static_cast<AsusACPIResponse *>(arguments->structureOutput));
}

IOReturn AsusACPIUserClient::getState(OSObject *target, void *, IOExternalMethodArguments *arguments)
{
    auto *client = OSDynamicCast(AsusACPIUserClient, target);
    if (!client || !client->service || !arguments->structureOutput) return kIOReturnNotAttached;
    return client->readState(*static_cast<AsusACPIState *>(arguments->structureOutput));
}

IOReturn AsusACPIUserClient::setMode(OSObject *target, void *, IOExternalMethodArguments *arguments)
{
    auto *client = OSDynamicCast(AsusACPIUserClient, target);
    if (!client || !client->service) return kIOReturnNotAttached;
    if (!arguments->scalarInput || arguments->scalarInput[0] > AsusACPIModeSilent) return kIOReturnBadArgument;
    return client->applyMode(static_cast<AsusACPIMode>(arguments->scalarInput[0]));
}

IOReturn AsusACPIUserClient::readState(AsusACPIState &state)
{
    auto *stateLock = service->stateLock;
    auto &active = service->active;
    auto *acpi = service->acpi;
    auto *fans = service->fans;
    auto &hasSetMode = service->hasSetMode;
    auto &lastSetMode = service->lastSetMode;

    if (!stateLock) return kIOReturnNotReady;
    IOLockLock(stateLock);
    if (!active) { IOLockUnlock(stateLock); return kIOReturnNotAttached; }
    bzero(&state, sizeof(state));
    state.version = AsusACPIProtocolVersion;
    bool supported = false;
    if (acpi->isSupported(AsusACPIPerformance::Profile, supported) == kIOReturnSuccess && supported) {
        state.capabilities |= AsusACPICapabilitySetMode;
    } else if (acpi->isSupported(AsusACPIPerformance::VivoBookProfile, supported) == kIOReturnSuccess && supported) {
        state.capabilities |= AsusACPICapabilitySetMode;
    }
    if (hasSetMode) {
        state.lastSetMode = static_cast<uint32_t>(lastSetMode);
        state.validFields |= AsusACPIValidLastSetMode;
    }
    if (fans->readSpeed(AsusFan::CPU, state.cpuFanRPM) == kIOReturnSuccess) {
        state.capabilities |= AsusACPICapabilityCPUFan;
        state.validFields |= AsusACPIValidCPUFan;
    }
    if (fans->readSpeed(AsusFan::GPU, state.gpuFanRPM) == kIOReturnSuccess) {
        state.capabilities |= AsusACPICapabilityGPUFan;
        state.validFields |= AsusACPIValidGPUFan;
    }
    IOLockUnlock(stateLock);
    return kIOReturnSuccess;
}

IOReturn AsusACPIUserClient::applyMode(AsusACPIMode mode)
{
    auto *stateLock = service->stateLock;
    auto &active = service->active;
    auto *performance = service->performance;
    auto &hasSetMode = service->hasSetMode;
    auto &lastSetMode = service->lastSetMode;

    if (!stateLock) return kIOReturnNotReady;
    IOLockLock(stateLock);
    if (!active) { IOLockUnlock(stateLock); return kIOReturnNotAttached; }
    uint32_t firmwareResult = 0;
    IOReturn status = performance->setMode(mode, firmwareResult);
    if (status == kIOReturnSuccess) { lastSetMode = mode; hasSetMode = true; }
    IOLockUnlock(stateLock);
    if (status == kIOReturnSuccess) KernelLog::info("Performance", "setMode", "mode=%u result=0x%08x", static_cast<unsigned>(mode), static_cast<unsigned>(firmwareResult));
    else KernelLog::error("Performance", "setMode", "mode=%u status=0x%08x result=0x%08x", static_cast<unsigned>(mode), static_cast<unsigned>(status), static_cast<unsigned>(firmwareResult));
    return status;
}

namespace {
bool decodePower(const AsusACPIPowerSettings &input, PowerConfiguration &config, AsusACPIModelLimits &limits)
{
    if (input.amdCPU > 1 || input.integratedOnly > 1 || input.maxTotal < 5 || input.maxTotal > 250 ||
        input.maxCPU < 5 || input.maxCPU > 115 || input.maxGPUBoost < 5 || input.maxGPUBoost > 25 ||
        input.maxCrossLoad > 125 || input.maxCPUTemperature < 75 || input.maxCPUTemperature > 103) return false;
    config.total = input.total; config.cpu = input.cpu; config.slow = input.slow; config.fast = input.fast;
    config.gpuToCpu = input.gpuToCpu; config.crossLoad = input.crossLoad; config.cpuTemperature = input.cpuTemperature;
    config.gpuBoost = input.gpuBoost; config.gpuTemperature = input.gpuTemperature; config.gpuAdditional = input.gpuAdditional;
    config.amdCPU = input.amdCPU != 0; config.integratedOnly = input.integratedOnly != 0;
    limits.maxTotal = input.maxTotal; limits.maxCPU = input.maxCPU; limits.maxGPUBoost = input.maxGPUBoost;
    limits.maxCrossLoad = input.maxCrossLoad; limits.maxCPUTemperature = input.maxCPUTemperature;
    return PowerController::isValid(config, limits);
}
}

IOReturn AsusACPIUserClient::dispatchController(const AsusACPIRequest &request, AsusACPIResponse &response)
{
    auto *stateLock = service->stateLock;
    auto &active = service->active;
    auto *acpi = service->acpi;
    auto *fans = service->fans;
    auto *performance = service->performance;
    auto &hasSetMode = service->hasSetMode;
    auto &lastSetMode = service->lastSetMode;

    if (!stateLock) return kIOReturnNotReady;
    if (request.payloadSize > sizeof(request.payload)) return kIOReturnBadArgument;
    IOLockLock(stateLock);
    if (!active) { IOLockUnlock(stateLock); return kIOReturnNotAttached; }
    bzero(&response, sizeof(response));
    const uint32_t *a = request.arguments;
    uint32_t &out = response.values[0];
    uint32_t result = 0;
    IOReturn status = kIOReturnUnsupported;
    PowerController power(*acpi);
    GraphicsController graphics(*acpi);
    DisplayController display(*acpi);
    KeyboardController keyboard(*acpi);
    DeviceController devices(*acpi);
    TouchpadController touchpad(service);
    auto fan = static_cast<AsusFan>(a[0]);
    switch (request.operation) {
    case AsusACPIOpPerformanceReadMode: {
        AsusACPIMode mode;
        status = performance->readMode(mode, response.values[1]);
        if (status == kIOReturnSuccess) out = static_cast<uint32_t>(mode);
        else {
            uint32_t profileRaw = 0;
            uint32_t vivoRaw = 0;
            IOReturn profileStatus = acpi->get(AsusACPIPerformance::Profile, profileRaw);
            IOReturn vivoStatus = acpi->get(AsusACPIPerformance::VivoBookProfile, vivoRaw);
            service->setProperty("PerformanceProfileStatus", static_cast<uint32_t>(profileStatus), 32);
            service->setProperty("PerformanceProfileRaw", profileRaw, 32);
            service->setProperty("PerformanceVivoStatus", static_cast<uint32_t>(vivoStatus), 32);
            service->setProperty("PerformanceVivoRaw", vivoRaw, 32);
            KernelLog::error("Performance", "readMode", "profile=0x%08x status=0x%08x vivo=0x%08x status=0x%08x", profileRaw, static_cast<unsigned>(profileStatus), vivoRaw, static_cast<unsigned>(vivoStatus));
        }
        break;
    }
    case AsusACPIOpPerformanceSetMode:
        if (a[0] > AsusACPIModeSilent) { status = kIOReturnBadArgument; break; }
        status = performance->setMode(static_cast<AsusACPIMode>(a[0]), out);
        if (status == kIOReturnSuccess) { lastSetMode = static_cast<AsusACPIMode>(a[0]); hasSetMode = true; }
        break;
    case AsusACPIOpPerformanceSetStatusMode:
        if (a[0] > AsusACPIModeSilent) { status = kIOReturnBadArgument; break; }
        status = performance->setStatusMode(static_cast<AsusACPIMode>(a[0]), out); break;
    case AsusACPIOpPerformanceApply: {
        if (request.payloadSize != sizeof(AsusACPIPerformanceSettings)) { status = kIOReturnBadArgument; break; }
        AsusACPIPerformanceSettings input;
        memcpy(&input, request.payload, sizeof(input));
        if (input.mode > AsusACPIModeSilent || input.applyStatusMode > 1 || input.manualMode > 1 || input.hasFans > 1 || input.hasMiddleFan > 1 || input.hasPower > 1 || input.hysteresisUp > 255 || input.hysteresisDown > 255 || (input.hasMiddleFan && !input.hasFans)) { status = kIOReturnBadArgument; break; }
        PerformanceConfiguration config;
        PowerConfiguration pc;
        config.mode = static_cast<AsusACPIMode>(input.mode); config.applyStatusMode = input.applyStatusMode; config.manualMode = input.manualMode;
        config.fanScale = input.fanScale; config.hysteresisUp = input.hysteresisUp; config.hysteresisDown = input.hysteresisDown;
        if (input.hasFans) { config.cpuCurve = input.cpuCurve; config.gpuCurve = input.gpuCurve; }
        if (input.hasMiddleFan) config.middleCurve = input.middleCurve;
        if (input.hasPower) {
            if (!decodePower(input.power, pc, config.limits)) { status = kIOReturnBadArgument; break; }
            config.power = &pc;
        }
        bool usedRange = false;
        status = performance->apply(config, usedRange);
        out = usedRange;
        if (status == kIOReturnSuccess) { lastSetMode = config.mode; hasSetMode = true; }
        break;
    }
    case AsusACPIOpFanReadSpeed: status = fans->readSpeed(fan, out); break;
    case AsusACPIOpFanReadTemperature: status = fans->readTemperature(fan, out); break;
    case AsusACPIOpFanReadCurve:
    case AsusACPIOpFanReadFloor: {
        size_t size = sizeof(response.payload);
        status = request.operation == AsusACPIOpFanReadCurve ? fans->readCurve(fan, a[1], response.payload, size) : fans->readFloor(fan, response.payload, size);
        response.payloadSize = static_cast<uint32_t>(size); break;
    }
    case AsusACPIOpFanReadHysteresis: {
        uint8_t up = 0, down = 0; status = fans->readHysteresis(up, down); out = up; response.values[1] = down; break;
    }
    case AsusACPIOpFanSetHysteresis:
        if (a[0] > 255 || a[1] > 255 || a[2] > 1) { status = kIOReturnBadArgument; break; }
        status = fans->setHysteresis(a[0], a[1], a[2] != 0, out); break;
    case AsusACPIOpFanSetCurve:
    case AsusACPIOpFanSetRange:
    case AsusACPIOpFanFixCurve: {
        if (request.payloadSize != 16) { status = kIOReturnBadArgument; break; }
        if (request.operation == AsusACPIOpFanFixCurve) {
            if (a[0] > 1) { status = kIOReturnBadArgument; break; }
            memcpy(response.payload, request.payload, 16); response.payloadSize = 16;
            status = fans->fixCurve(response.payload, a[0] != 0);
        } else if (request.operation == AsusACPIOpFanSetCurve) status = fans->setCurve(fan, request.payload, a[1], out);
        else status = fans->setRange(fan, request.payload, out);
        break;
    }
    case AsusACPIOpFanApplyCurves: {
        if ((request.payloadSize != 32 && request.payloadSize != 48) || a[1] > AsusACPIModeSilent) { status = kIOReturnBadArgument; break; }
        bool usedRange = false;
        status = fans->applyCurves(request.payload, request.payload + 16, request.payloadSize == 48 ? request.payload + 32 : nullptr, a[0], static_cast<AsusACPIMode>(a[1]), usedRange);
        out = usedRange; break;
    }
    case AsusACPIOpPowerRead: status = power.read(static_cast<PowerFeature>(a[0]), out); break;
    case AsusACPIOpPowerWrite: status = power.write(static_cast<PowerFeature>(a[0]), a[1], out); break;
    case AsusACPIOpPowerReadBatteryDischarge: {
        int16_t value = 0; status = power.readBatteryDischarge(value); out = static_cast<uint32_t>(static_cast<int32_t>(value)); break;
    }
    case AsusACPIOpPowerApply: {
        if (request.payloadSize != sizeof(AsusACPIPowerSettings)) { status = kIOReturnBadArgument; break; }
        AsusACPIPowerSettings input; memcpy(&input, request.payload, sizeof(input));
        PowerConfiguration config; AsusACPIModelLimits limits;
        if (!decodePower(input, config, limits)) { status = kIOReturnBadArgument; break; }
        status = power.apply(config, limits); break;
    }
    case AsusACPIOpGraphicsRead: status = graphics.read(static_cast<GraphicsFeature>(a[0]), out); break;
    case AsusACPIOpGraphicsWrite: status = graphics.write(static_cast<GraphicsFeature>(a[0]), a[1], out); break;
    case AsusACPIOpGraphicsReadApuMemory: status = graphics.readApuMemory(out); break;
    case AsusACPIOpGraphicsReadVramOptions: {
        uint16_t options[80] = {}; size_t count = 80;
        status = graphics.readVramOptions(options, count, response.values[0], response.values[1]);
        if (status == kIOReturnSuccess) { response.values[2] = count; response.payloadSize = count * sizeof(uint16_t); memcpy(response.payload, options, response.payloadSize); }
        break;
    }
    case AsusACPIOpGraphicsReadVramMemory: status = graphics.readVramMemory(out); break;
    case AsusACPIOpGraphicsSetApuMemory: status = graphics.setApuMemory(a[0], out); break;
    case AsusACPIOpGraphicsExternalDisplay:
    case AsusACPIOpGraphicsExternalGPU: {
        bool connected = false;
        status = request.operation == AsusACPIOpGraphicsExternalDisplay ? graphics.externalDisplayConnected(connected) : graphics.externalGPUConnected(connected);
        out = connected; break;
    }
    case AsusACPIOpGraphicsReadMode: {
        GraphicsMode mode;
        status = graphics.readMode(static_cast<GraphicsFeature>(a[0]), static_cast<GraphicsFeature>(a[1]), mode);
        if (status == kIOReturnSuccess) out = static_cast<uint32_t>(mode);
        break;
    }
    case AsusACPIOpGraphicsSetMode: {
        bool restart = false;
        status = graphics.setMode(static_cast<GraphicsFeature>(a[0]), static_cast<GraphicsFeature>(a[1]), static_cast<GraphicsMode>(a[2]), restart);
        out = restart; break;
    }
    case AsusACPIOpGraphicsIsAllAmdPpt:
    case AsusACPIOpGraphicsIsNvidiaGPU: {
        bool supported = false;
        if ((request.operation == AsusACPIOpGraphicsIsAllAmdPpt && a[0] > 1) || (request.operation == AsusACPIOpGraphicsIsNvidiaGPU && (a[1] > 1 || a[2] > 1))) { status = kIOReturnBadArgument; break; }
        status = request.operation == AsusACPIOpGraphicsIsAllAmdPpt ? graphics.isAllAmdPpt(a[0], supported) : graphics.isNvidiaGPU(static_cast<GraphicsFeature>(a[0]), a[1], a[2], supported);
        out = supported; break;
    }
    case AsusACPIOpDisplayRead: status = display.read(static_cast<DisplayFeature>(a[0]), out); break;
    case AsusACPIOpDisplayWrite: status = display.write(static_cast<DisplayFeature>(a[0]), a[1], out); break;
    case AsusACPIOpDisplaySupportsOverdrive: { bool supported = false; status = display.supportsOverdrive(supported); out = supported; break; }
    case AsusACPIOpDisplaySetMiniLed: status = display.setMiniLed(a[0], out); break;
    case AsusACPIOpDisplaySetScreenPadBrightness: status = display.setScreenPadBrightness(static_cast<int32_t>(a[0]), out); break;
    case AsusACPIOpKeyboardReadFnLock: status = keyboard.readFnLock(out); break;
    case AsusACPIOpKeyboardSetFnLock:
        if (a[0] > 1) { status = kIOReturnBadArgument; break; }
        status = keyboard.setFnLock(a[0], out); break;
    case AsusACPIOpKeyboardSetBrightness:
        if (a[0] > 0x7f) { status = kIOReturnBadArgument; break; }
        status = keyboard.setBrightness(static_cast<uint8_t>(a[0]), out); break;
    case AsusACPIOpKeyboardSetRGB:
        if (a[0] > 255 || a[1] > 255 || a[2] > 255 || a[3] > 255 || a[4] > 255) { status = kIOReturnBadArgument; break; }
        status = keyboard.setRGB(a[0], a[1], a[2], a[3], a[4], out); break;
    case AsusACPIOpKeyboardSetPower:
        if (a[0] > 1 || a[1] > 1 || a[2] > 1 || a[3] > 1 || a[4] > 1) { status = kIOReturnBadArgument; break; }
        status = keyboard.setPower(a[0], a[1], a[2], a[3], a[4], out); break;
    case AsusACPIOpKeyboardSendFirmwareKey: status = keyboard.sendFirmwareKey(a[0], out); break;
    case AsusACPIOpDeviceRead: status = devices.read(static_cast<DeviceFeature>(a[0]), out); break;
    case AsusACPIOpDeviceWrite: status = devices.write(static_cast<DeviceFeature>(a[0]), a[1], out); break;
    case AsusACPIOpDeviceReadCores: {
        uint8_t efficiency = 0, performanceCores = 0;
        status = devices.readCores(static_cast<DeviceFeature>(a[0]), efficiency, performanceCores);
        out = efficiency; response.values[1] = performanceCores; break;
    }
    case AsusACPIOpDeviceSetCores:
        if (a[0] > 255 || a[1] > 255) { status = kIOReturnBadArgument; break; }
        status = devices.setCores(a[0], a[1], out); break;
    case AsusACPIOpDeviceToggleCameraShutter: {
        bool enabled = false; status = devices.toggleCameraShutter(enabled, result); out = enabled; response.values[1] = result; break;
    }
    case AsusACPIOpDeviceSetStatusLed:
        if (a[0] > 1) { status = kIOReturnBadArgument; break; }
        status = devices.setStatusLed(a[0], out); break;
    case AsusACPIOpTouchpadRead: {
        bool enabled = false;
        status = touchpad.read(enabled);
        out = enabled;
        break;
    }
    case AsusACPIOpTouchpadSet:
        if (a[0] > 1) { status = kIOReturnBadArgument; break; }
        status = touchpad.set(a[0] != 0);
        break;
    default: break;
    }
    IOLockUnlock(stateLock);
    if (status != kIOReturnSuccess && status != kIOReturnUnsupported) {
        KernelLog::error("UserClient", "controller", "operation=%u status=0x%08x", static_cast<unsigned>(request.operation), static_cast<unsigned>(status));
    }
    return status;
}
