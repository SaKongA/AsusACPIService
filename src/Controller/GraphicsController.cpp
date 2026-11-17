#include "GraphicsController.hpp"
#include <IOKit/IOLib.h>

GraphicsController::GraphicsController(AsusACPI &acpi) : acpi(acpi) {}

IOReturn GraphicsController::read(GraphicsFeature feature, uint32_t &value)
{
    switch (feature) {
    case GraphicsFeature::RogEco:
    case GraphicsFeature::VivoEco:
    case GraphicsFeature::ExternalGpuConnected:
    case GraphicsFeature::ExternalGpu:
    case GraphicsFeature::ExternalDisplay:
    case GraphicsFeature::RogMux:
    case GraphicsFeature::VivoMux:
    case GraphicsFeature::ApuMemory:
    case GraphicsFeature::VramMemory:
        return acpi.getPresentValue(static_cast<uint32_t>(feature), value);
    }
    return kIOReturnBadArgument;
}

IOReturn GraphicsController::write(GraphicsFeature feature, uint32_t value, uint32_t &firmwareResult)
{
    switch (feature) {
    case GraphicsFeature::RogEco:
    case GraphicsFeature::VivoEco:
    case GraphicsFeature::ExternalGpu:
    case GraphicsFeature::RogMux:
    case GraphicsFeature::VivoMux:
    case GraphicsFeature::ApuMemory:
    case GraphicsFeature::VramMemory:
        break;
    default:
        return kIOReturnUnsupported;
    }
    firmwareResult = 0;
    IOReturn status = acpi.set(static_cast<uint32_t>(feature), value, firmwareResult);
    if (status != kIOReturnSuccess) return status;
    return firmwareResult == 1 ? kIOReturnSuccess : kIOReturnError;
}

IOReturn GraphicsController::readApuMemory(uint32_t &index)
{
    return AsusACPIGraphics::readApuMemory(acpi, index);
}

IOReturn GraphicsController::readVramOptions(uint16_t *options, size_t &count, uint32_t &unitMB, uint32_t &current)
{
    return AsusACPIGraphics::readVramOptions(acpi, options, count, unitMB, current);
}

IOReturn GraphicsController::readVramMemory(uint32_t &current)
{
    return AsusACPIGraphics::readVramMemory(acpi, current);
}

IOReturn GraphicsController::setApuMemory(uint32_t index, uint32_t &firmwareResult)
{
    firmwareResult = 0;
    IOReturn status = AsusACPIGraphics::setApuMemory(acpi, index, firmwareResult);
    if (status != kIOReturnSuccess) return status;
    return firmwareResult == 1 ? kIOReturnSuccess : kIOReturnError;
}

IOReturn GraphicsController::externalDisplayConnected(bool &connected)
{
    return AsusACPIGraphics::externalDisplayConnected(acpi, connected);
}

IOReturn GraphicsController::externalGPUConnected(bool &connected)
{
    return AsusACPIGraphics::externalGPUConnected(acpi, connected);
}

IOReturn GraphicsController::readMode(GraphicsFeature ecoFeature, GraphicsFeature muxFeature, GraphicsMode &mode)
{
    if ((ecoFeature != GraphicsFeature::RogEco && ecoFeature != GraphicsFeature::VivoEco) ||
        (muxFeature != GraphicsFeature::RogMux && muxFeature != GraphicsFeature::VivoMux)) return kIOReturnBadArgument;
    uint32_t eco = 0, mux = 0;
    IOReturn ecoStatus = read(ecoFeature, eco);
    IOReturn muxStatus = read(muxFeature, mux);
    if (muxStatus == kIOReturnSuccess && mux == 0) { mode = GraphicsMode::Ultimate; return kIOReturnSuccess; }
    if (ecoStatus == kIOReturnSuccess && eco <= 1) { mode = eco == 1 ? GraphicsMode::Eco : GraphicsMode::Standard; return kIOReturnSuccess; }
    return ecoStatus != kIOReturnSuccess ? ecoStatus : kIOReturnUnsupported;
}

IOReturn GraphicsController::setMode(GraphicsFeature ecoFeature, GraphicsFeature muxFeature, GraphicsMode target, bool &restartRequired)
{
    restartRequired = false;
    if (target != GraphicsMode::Eco && target != GraphicsMode::Standard && target != GraphicsMode::Ultimate) return kIOReturnBadArgument;
    GraphicsMode current;
    IOReturn status = readMode(ecoFeature, muxFeature, current);
    if (status != kIOReturnSuccess || current == target) return status;
    uint32_t result = 0;
    if (current == GraphicsMode::Ultimate) {
        status = write(muxFeature, 1, result);
        if (status == kIOReturnSuccess) restartRequired = true;
        return status;
    }
    if (target == GraphicsMode::Ultimate) {
        uint32_t mux = 0;
        status = read(muxFeature, mux);
        if (status != kIOReturnSuccess) return status;
        if (current == GraphicsMode::Eco) {
            status = write(ecoFeature, 0, result);
            if (status != kIOReturnSuccess) return status;
            IOSleep(500);
            uint32_t eco = 0;
            status = read(ecoFeature, eco);
            if (status != kIOReturnSuccess) return status;
            if (eco != 0) return kIOReturnNotReady;
        }
        status = write(muxFeature, 0, result);
        if (status == kIOReturnSuccess) restartRequired = true;
        return status;
    }
    // TODO: Windows GPU process and driver service management needs a macOS equivalent.
    return write(ecoFeature, target == GraphicsMode::Eco ? 1 : 0, result);
}

IOReturn GraphicsController::isAllAmdPpt(bool integratedOnly, bool &supported)
{
    return AsusACPIGraphics::isAllAmdPpt(acpi, integratedOnly, supported);
}

IOReturn GraphicsController::isNvidiaGPU(GraphicsFeature ecoFeature, bool integratedOnly, bool ally, bool &supported)
{
    return AsusACPIGraphics::isNvidiaGPU(acpi, static_cast<uint32_t>(ecoFeature), integratedOnly, ally, supported);
}

