#include "Graphics.hpp"
#include "Power.hpp"

namespace {
constexpr uint32_t ApuMemoryValues[] = {0, 2, 3, 4, 5, 7, 8, 9, 6};
}

IOReturn AsusACPIGraphics::setEco(AsusACPI &acpi, uint32_t device, bool enabled, uint32_t &result)
{
    if (device != RogEco && device != VivoEco) return kIOReturnBadArgument;
    uint32_t current = 0;
    IOReturn status = acpi.getPresentValue(device, current);
    if (status != kIOReturnSuccess) return status;
    if (current > 1) return kIOReturnBadMedia;
    if (current == static_cast<uint32_t>(enabled)) {
        result = 0;
        return kIOReturnSuccess;
    }
    return acpi.set(device, enabled ? 1 : 0, result);
}

IOReturn AsusACPIGraphics::externalDisplayConnected(AsusACPI &acpi, bool &connected)
{
    uint32_t value = 0;
    IOReturn status = acpi.getPresentValue(ExternalDisplay, value);
    if (status != kIOReturnSuccess) return status;
    connected = ((value >> 4) & value & 0xF) != 0;
    return kIOReturnSuccess;
}

IOReturn AsusACPIGraphics::externalGPUConnected(AsusACPI &acpi, bool &connected)
{
    uint32_t value = 0;
    IOReturn status = acpi.getPresentValue(ExternalGPUConnected, value);
    if (status != kIOReturnSuccess) return status;
    connected = value == 1;
    return kIOReturnSuccess;
}

IOReturn AsusACPIGraphics::setApuMemory(AsusACPI &acpi, uint32_t index, uint32_t &result)
{
    if (index >= sizeof(ApuMemoryValues) / sizeof(ApuMemoryValues[0])) return kIOReturnBadArgument;
    return acpi.set(ApuMemory, index == 0 ? 0 : 0x100u | ApuMemoryValues[index], result);
}

IOReturn AsusACPIGraphics::readApuMemory(AsusACPI &acpi, uint32_t &index)
{
    uint32_t value = 0;
    IOReturn status = acpi.getPresentValue(ApuMemory, value);
    if (status != kIOReturnSuccess) return status;
    for (uint32_t i = 0; i < sizeof(ApuMemoryValues) / sizeof(ApuMemoryValues[0]); ++i) {
        if (value == (0x100u | ApuMemoryValues[i])) { index = i; return kIOReturnSuccess; }
    }
    index = 4;
    return kIOReturnSuccess;
}

IOReturn AsusACPIGraphics::readVramOptions(AsusACPI &acpi, uint16_t *options, size_t &count, uint32_t &unitMB, uint32_t &current)
{
    // TODO: Verify the macOS ACPI buffer layout against the Windows driver output.
    if (!options) return kIOReturnBadArgument;
    uint8_t data[64] = {};
    size_t size = sizeof(data);
    IOReturn status = acpi.getBuffer(VramMemory, 0, data, size);
    if (status != kIOReturnSuccess) return status;
    if (size < 8) return kIOReturnBadMedia;
    uint32_t header = static_cast<uint32_t>(data[0]) | (static_cast<uint32_t>(data[1]) << 8) | (static_cast<uint32_t>(data[2]) << 16) | (static_cast<uint32_t>(data[3]) << 24);
    if ((header & 0x10000u) == 0 || (header & 0x80000u) != 0) return kIOReturnUnsupported;
    current = static_cast<uint32_t>(data[4]) | (static_cast<uint32_t>(data[5]) << 8) | (static_cast<uint32_t>(data[6]) << 16) | (static_cast<uint32_t>(data[7]) << 24);
    size_t available = (size - 6) / 2;
    size_t required = header & 0xFFFFu;
    if (required > available) required = available;
    if (required < 2) return kIOReturnBadMedia;
    if (count < required) { count = required; return kIOReturnNoSpace; }
    options[0] = 0;
    for (size_t i = 1; i < required; ++i) options[i] = static_cast<uint16_t>(data[6 + i * 2] | (data[7 + i * 2] << 8));
    count = required;
    unitMB = (header & 0x20000u) ? 512 : 1;
    return kIOReturnSuccess;
}

IOReturn AsusACPIGraphics::readVramMemory(AsusACPI &acpi, uint32_t &current)
{
    uint8_t data[64] = {};
    size_t size = sizeof(data);
    IOReturn status = acpi.getBuffer(VramMemory, 0, data, size);
    if (status != kIOReturnSuccess) return status;
    if (size < 8) return kIOReturnBadMedia;
    current = static_cast<uint32_t>(data[4]) | (static_cast<uint32_t>(data[5]) << 8) | (static_cast<uint32_t>(data[6]) << 16) | (static_cast<uint32_t>(data[7]) << 24);
    return kIOReturnSuccess;
}

IOReturn AsusACPIGraphics::setVramMemory(AsusACPI &acpi, uint32_t value, uint32_t &result)
{
    return acpi.set(VramMemory, value, result);
}

IOReturn AsusACPIGraphics::isAllAmdPpt(AsusACPI &acpi, bool integratedOnly, bool &supported)
{
    bool cpuPpt = false;
    IOReturn status = acpi.isSupported(AsusACPIPower::CPUPpt, cpuPpt);
    if (status != kIOReturnSuccess) return status;
    if (!cpuPpt || integratedOnly) { supported = false; return kIOReturnSuccess; }
    bool gpuBoost = false;
    status = acpi.isSupported(AsusACPIPower::GPUBoost, gpuBoost);
    if (status != kIOReturnSuccess) return status;
    supported = !gpuBoost;
    return kIOReturnSuccess;
}

IOReturn AsusACPIGraphics::isNvidiaGPU(AsusACPI &acpi, uint32_t ecoDevice, bool integratedOnly, bool ally, bool &supported)
{
    if (ecoDevice != RogEco && ecoDevice != VivoEco) return kIOReturnBadArgument;
    bool allAmd = false;
    IOReturn status = isAllAmdPpt(acpi, integratedOnly, allAmd);
    if (status != kIOReturnSuccess) return status;
    if (allAmd || ally) { supported = false; return kIOReturnSuccess; }
    return acpi.isSupported(ecoDevice, supported);
}
