#include "Fans.hpp"

IOReturn AsusACPIFans::readSpeed(AsusACPI &acpi, uint32_t device, uint32_t &rpm)
{
    if (device != CPUSpeed && device != GPUSpeed && device != MiddleSpeed) return kIOReturnBadArgument;

    uint32_t value = 0;
    IOReturn status = acpi.getPresentValue(device, value);
    if (status != kIOReturnSuccess) return status;
    value &= 0xFFFFu;
    if (value > 120) return kIOReturnBadMedia;

    rpm = value * 100;
    return kIOReturnSuccess;
}

IOReturn AsusACPIFans::readCPUSpeed(AsusACPI &acpi, uint32_t &rpm)
{
    return readSpeed(acpi, CPUSpeed, rpm);
}

IOReturn AsusACPIFans::readGPUSpeed(AsusACPI &acpi, uint32_t &rpm)
{
    return readSpeed(acpi, GPUSpeed, rpm);
}

IOReturn AsusACPIFans::readMiddleSpeed(AsusACPI &acpi, uint32_t &rpm)
{
    return readSpeed(acpi, MiddleSpeed, rpm);
}

IOReturn AsusACPIFans::readCurve(AsusACPI &acpi, uint32_t device, uint32_t mode, void *output, size_t &outputSize)
{
    // TODO: Verify firmware buffer layout against the target model.
    if ((device != CPUCurve && device != GPUCurve && device != MiddleCurve) || mode > 2) return kIOReturnBadArgument;
    uint32_t mapped = mode == 1 ? 2 : mode == 2 ? 1 : 0;
    return acpi.getBuffer(device, mapped, output, outputSize);
}

IOReturn AsusACPIFans::readFloor(AsusACPI &acpi, uint32_t device, void *output, size_t &outputSize)
{
    if (device != CPUFloor && device != GPUFloor && device != MiddleFloor) return kIOReturnBadArgument;
    return acpi.getBuffer(device, 0, output, outputSize);
}

IOReturn AsusACPIFans::readHysteresis(AsusACPI &acpi, uint8_t &up, uint8_t &down)
{
    uint32_t value = 0;
    IOReturn status = acpi.getPresentValue(Hysteresis, value);
    if (status != kIOReturnSuccess) return status;
    up = static_cast<uint8_t>(value);
    down = static_cast<uint8_t>(value >> 8);
    return kIOReturnSuccess;
}

IOReturn AsusACPIFans::writeHysteresis(AsusACPI &acpi, uint8_t up, uint8_t down, bool middleFan, uint32_t &result)
{
    bool supported = false;
    IOReturn status = acpi.isSupported(Hysteresis, supported);
    if (status != kIOReturnSuccess) return status;
    if (!supported) return kIOReturnUnsupported;
    uint8_t payload[16] = {};
    for (unsigned i = 0; i < (middleFan ? 3u : 2u); ++i) {
        payload[i * 4] = up;
        payload[i * 4 + 1] = down;
    }
    return acpi.setBuffer(Hysteresis, payload, sizeof(payload), result);
}

IOReturn AsusACPIFans::writeCurve(AsusACPI &acpi, uint32_t device, const uint8_t curve[16], uint32_t &result, uint32_t scale)
{
    if (!curve || isEmptyCurve(curve) || scale > 255 || (device != CPUCurve && device != GPUCurve && device != MiddleCurve)) return kIOReturnBadArgument;
    uint8_t payload[16];
    for (unsigned i = 0; i < 8; ++i) {
        payload[i] = curve[i];
        uint32_t duty = curve[i + 8] > 100 ? 100 : curve[i + 8];
        uint32_t scaled = duty * scale / 100;
        if (scaled > 255) return kIOReturnBadArgument;
        payload[i + 8] = static_cast<uint8_t>(scaled);
    }
    return acpi.setBuffer(device, payload, 16, result);
}

IOReturn AsusACPIFans::writeRange(AsusACPI &acpi, uint32_t device, const uint8_t curve[16], uint32_t &result)
{
    if (!curve || isEmptyCurve(curve) || (device != CPURange && device != GPURange)) return kIOReturnBadArgument;
    uint8_t payload[] = {static_cast<uint8_t>(curve[8] * 255 / 100), static_cast<uint8_t>(curve[15] * 255 / 100)};
    return acpi.setBuffer(device, payload, sizeof(payload), result);
}

bool AsusACPIFans::isEmptyCurve(const uint8_t curve[16])
{
    if (!curve) return true;
    for (unsigned i = 0; i < 16; ++i) if (curve[i] != 0) return false;
    return true;
}

IOReturn AsusACPIFans::fixCurve(uint8_t curve[16], bool clampPoints)
{
    if (!curve) return kIOReturnBadArgument;
    uint8_t temperatures[8] = {};
    uint8_t duties[8] = {};
    unsigned count = 0;
    unsigned previous = 0;
    for (unsigned i = 0; i < 8; ++i) {
        unsigned temperature = curve[i];
        if (temperature <= previous) temperature = previous + 6 > 100 ? 100 : previous + 6;
        previous = temperature;
        unsigned index = 0;
        while (index < count && temperatures[index] < temperature) ++index;
        if (index < count && temperatures[index] == temperature) {
            duties[index] = curve[i + 8];
        } else {
            for (unsigned j = count; j > index; --j) { temperatures[j] = temperatures[j - 1]; duties[j] = duties[j - 1]; }
            temperatures[index] = static_cast<uint8_t>(temperature);
            duties[index] = curve[i + 8];
            ++count;
        }
    }
    bool prepend = count > 0 && temperatures[0] >= 40;
    unsigned output = 0;
    if (prepend) { curve[0] = 30; curve[8] = 0; output = 1; }
    for (unsigned i = 0; i < count && output < 8; ++i) {
        if (prepend && i == 3) continue;
        unsigned temperature = temperatures[i];
        if (clampPoints) {
            unsigned minimum = 30 + output * 10;
            unsigned maximum = minimum + 10;
            if (temperature < minimum) temperature = minimum;
            if (temperature > maximum) temperature = maximum;
        }
        curve[output] = static_cast<uint8_t>(temperature);
        curve[output + 8] = duties[i];
        ++output;
    }
    return kIOReturnSuccess;
}
