#ifndef GRAPHICS_CONTROLLER_HPP
#define GRAPHICS_CONTROLLER_HPP

#include "../ACPI/Graphics.hpp"

enum class GraphicsFeature : uint32_t {
    RogEco = AsusACPIGraphics::RogEco,
    VivoEco = AsusACPIGraphics::VivoEco,
    ExternalGpuConnected = AsusACPIGraphics::ExternalGPUConnected,
    ExternalGpu = AsusACPIGraphics::ExternalGPU,
    ExternalDisplay = AsusACPIGraphics::ExternalDisplay,
    RogMux = AsusACPIGraphics::RogMux,
    VivoMux = AsusACPIGraphics::VivoMux,
    ApuMemory = AsusACPIGraphics::ApuMemory,
    VramMemory = AsusACPIGraphics::VramMemory
};

enum class GraphicsMode {
    Eco,
    Standard,
    Ultimate
};

class GraphicsController {
public:
    explicit GraphicsController(AsusACPI &acpi);
    IOReturn read(GraphicsFeature feature, uint32_t &value);
    IOReturn write(GraphicsFeature feature, uint32_t value, uint32_t &firmwareResult);
    IOReturn readApuMemory(uint32_t &index);
    IOReturn readVramOptions(uint16_t *options, size_t &count, uint32_t &unitMB, uint32_t &current);
    IOReturn readVramMemory(uint32_t &current);
    IOReturn setApuMemory(uint32_t index, uint32_t &firmwareResult);
    IOReturn externalDisplayConnected(bool &connected);
    IOReturn externalGPUConnected(bool &connected);
    IOReturn readMode(GraphicsFeature ecoFeature, GraphicsFeature muxFeature, GraphicsMode &mode);
    IOReturn setMode(GraphicsFeature ecoFeature, GraphicsFeature muxFeature, GraphicsMode mode, bool &restartRequired);
    IOReturn isAllAmdPpt(bool integratedOnly, bool &supported);
    IOReturn isNvidiaGPU(GraphicsFeature ecoFeature, bool integratedOnly, bool ally, bool &supported);

private:
    AsusACPI &acpi;
};

#endif
