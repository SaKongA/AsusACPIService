#ifndef ACPI_TRANSPORT_HPP
#define ACPI_TRANSPORT_HPP

#include "AsusACPI.hpp"
#include <IOKit/IOLocks.h>
#include <IOKit/acpi/IOACPIPlatformDevice.h>

class ACPITransport final : public AsusACPITransport {
public:
    ACPITransport();
    ~ACPITransport() override;
    bool attach(IOACPIPlatformDevice *device);
    IOReturn evaluate(uint32_t method, const uint32_t *arguments, uint32_t argumentCount, uint32_t &result) override;
    IOReturn evaluateBuffer(uint32_t method, const void *arguments, size_t argumentSize, void *output, size_t &outputSize) override;

private:
    IOReturn call(uint32_t method, const void *arguments, size_t argumentSize, OSObject **result);
    IOACPIPlatformDevice *device;
    IOLock *lock;
};

#endif
