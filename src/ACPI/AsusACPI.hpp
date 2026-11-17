#ifndef ASUS_ACPI_HPP
#define ASUS_ACPI_HPP

#include <IOKit/IOReturn.h>
#include <IOKit/IOLocks.h>
#include <stddef.h>
#include <stdint.h>

class AsusACPITransport {
public:
    virtual IOReturn evaluate(uint32_t method, const uint32_t *arguments, uint32_t argumentCount, uint32_t &result) = 0;
    virtual IOReturn evaluateBuffer(uint32_t method, const void *arguments, size_t argumentSize, void *output, size_t &outputSize) = 0;
    virtual ~AsusACPITransport() = default;
};

class AsusACPI {
public:
    using ScanCallback = bool (*)(uint32_t device, IOReturn status, const uint8_t *data, size_t size, void *context);
    explicit AsusACPI(AsusACPITransport &transport);
    ~AsusACPI();
    AsusACPI(const AsusACPI &) = delete;
    AsusACPI &operator=(const AsusACPI &) = delete;
    IOReturn initialize(uint32_t &result);
    IOReturn watchdog(uint32_t &result);
    IOReturn get(uint32_t device, uint32_t &result);
    IOReturn getPresentValue(uint32_t device, uint32_t &value);
    IOReturn isSupported(uint32_t device, bool &supported);
    void clearSupportCache();
    IOReturn set(uint32_t device, uint32_t value, uint32_t &result);
    IOReturn getBuffer(uint32_t device, uint32_t value, void *output, size_t &outputSize);
    IOReturn setBuffer(uint32_t device, const void *payload, size_t payloadSize, uint32_t &result);
    IOReturn scanRange(ScanCallback callback, void *context);

private:
    AsusACPITransport &transport;
    struct SupportEntry {
        uint32_t device;
        bool supported;
        bool valid;
    };
    static constexpr size_t SupportCacheSize = 128;
    SupportEntry supportCache[SupportCacheSize] = {};
    size_t supportCacheNext = 0;
    IOLock *supportCacheLock;
};

// TODO: Replace Windows WMI AsusAtkWmiEvent subscription and ATK4001 event listener with a macOS event source.
// TODO: Windows DeviceSetWmi is a Windows-only fallback and has no direct macOS implementation.

#endif
