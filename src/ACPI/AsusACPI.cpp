#include "AsusACPI.hpp"
#include <libkern/libkern.h>

namespace {
constexpr uint32_t MethodInit = 0x54494E49;
constexpr uint32_t MethodWatchdog = 0x474F4457;
constexpr uint32_t MethodGet = 0x53545344;
constexpr uint32_t MethodSet = 0x53564544;
}

AsusACPI::AsusACPI(AsusACPITransport &transport) : transport(transport), supportCacheLock(IOLockAlloc()) {}

AsusACPI::~AsusACPI()
{
    if (supportCacheLock) IOLockFree(supportCacheLock);
}

IOReturn AsusACPI::initialize(uint32_t &result)
{
    const uint32_t arguments[] = {0, 0};
    return transport.evaluate(MethodInit, arguments, 2, result);
}

IOReturn AsusACPI::watchdog(uint32_t &result)
{
    const uint32_t arguments[] = {0, 0};
    return transport.evaluate(MethodWatchdog, arguments, 2, result);
}

IOReturn AsusACPI::get(uint32_t device, uint32_t &result)
{
    const uint32_t arguments[] = {device, 0};
    return transport.evaluate(MethodGet, arguments, 2, result);
}

IOReturn AsusACPI::getPresentValue(uint32_t device, uint32_t &value)
{
    uint32_t raw = 0;
    IOReturn status = get(device, raw);
    if (status != kIOReturnSuccess) return status;
    uint32_t decoded = raw - 0x00010000u;
    if (decoded > INT32_MAX) return kIOReturnUnsupported;
    value = decoded;
    return kIOReturnSuccess;
}

IOReturn AsusACPI::set(uint32_t device, uint32_t value, uint32_t &result)
{
    const uint32_t arguments[] = {device, value};
    return transport.evaluate(MethodSet, arguments, 2, result);
}

IOReturn AsusACPI::isSupported(uint32_t device, bool &supported)
{
    if (supportCacheLock) {
        IOLockLock(supportCacheLock);
        for (size_t i = 0; i < SupportCacheSize; ++i) {
            if (supportCache[i].valid && supportCache[i].device == device) {
                supported = supportCache[i].supported;
                IOLockUnlock(supportCacheLock);
                return kIOReturnSuccess;
            }
        }
        IOLockUnlock(supportCacheLock);
    }
    uint32_t raw = 0;
    IOReturn status = get(device, raw);
    if (status != kIOReturnSuccess) return status;
    supported = (raw - 0x00010000u) <= INT32_MAX;
    if (supportCacheLock) {
        IOLockLock(supportCacheLock);
        supportCache[supportCacheNext] = {device, supported, true};
        supportCacheNext = (supportCacheNext + 1) % SupportCacheSize;
        IOLockUnlock(supportCacheLock);
    }
    return kIOReturnSuccess;
}

void AsusACPI::clearSupportCache()
{
    if (!supportCacheLock) return;
    IOLockLock(supportCacheLock);
    for (size_t i = 0; i < SupportCacheSize; ++i) supportCache[i].valid = false;
    supportCacheNext = 0;
    IOLockUnlock(supportCacheLock);
}

IOReturn AsusACPI::getBuffer(uint32_t device, uint32_t value, void *output, size_t &outputSize)
{
    if (!output || outputSize == 0) return kIOReturnBadArgument;
    const uint32_t arguments[] = {device, value};
    return transport.evaluateBuffer(MethodGet, arguments, sizeof(arguments), output, outputSize);
}

IOReturn AsusACPI::setBuffer(uint32_t device, const void *payload, size_t payloadSize, uint32_t &result)
{
    if (!payload || payloadSize == 0 || payloadSize > 16) return kIOReturnBadArgument;
    uint8_t arguments[20] = {};
    memcpy(arguments, &device, sizeof(device));
    memcpy(arguments + sizeof(device), payload, payloadSize);
    size_t outputSize = sizeof(result);
    IOReturn status = transport.evaluateBuffer(MethodSet, arguments, sizeof(device) + payloadSize, &result, outputSize);
    if (status != kIOReturnSuccess) return status;
    return outputSize == sizeof(result) ? kIOReturnSuccess : kIOReturnBadMedia;
}

IOReturn AsusACPI::scanRange(ScanCallback callback, void *context)
{
    if (!callback) return kIOReturnBadArgument;
    (void)context;
    // TODO: Implement device scanning and report formatting outside the kernel.
    return kIOReturnUnsupported;
}
