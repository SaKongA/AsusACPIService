#include "ACPITransport.hpp"
#include "../Diagnostics/KernelLog.hpp"
#include <libkern/c++/OSData.h>
#include <libkern/c++/OSNumber.h>
#include <libkern/libkern.h>

namespace {
// TODO: Verify WMNB argument size and output types against the target firmware.
constexpr uint32_t ReadMethod = 0x53545344;
constexpr uint32_t WriteMethod = 0x53564544;
constexpr uint32_t InitializeMethod = 0x54494E49;
constexpr uint32_t WatchdogMethod = 0x474F4457;
constexpr size_t ArgumentSize = 5 * sizeof(uint32_t);
}

ACPITransport::ACPITransport() : device(nullptr), lock(nullptr) {}

ACPITransport::~ACPITransport()
{
    if (device) device->release();
    if (lock) IOLockFree(lock);
}

bool ACPITransport::attach(IOACPIPlatformDevice *provider)
{
    if (!provider || device) { KernelLog::error("ACPI", "attach", "invalid provider or transport already attached"); return false; }
    if (provider->validateObject("WMNB") != kIOReturnSuccess) { KernelLog::error("ACPI", "attach", "WMNB method not found"); return false; }
    lock = IOLockAlloc();
    if (!lock) { KernelLog::error("ACPI", "attach", "failed to allocate transport lock"); return false; }
    provider->retain();
    device = provider;
    KernelLog::info("ACPI", "attach", "WMNB method available");
    return true;
}

IOReturn ACPITransport::call(uint32_t method, const void *arguments, size_t argumentSize, OSObject **result)
{
    if (!device || !lock) return kIOReturnNotAttached;
    if (method != ReadMethod && method != WriteMethod && method != InitializeMethod && method != WatchdogMethod) return kIOReturnUnsupported;
    if (!result || argumentSize > ArgumentSize || (argumentSize && !arguments)) return kIOReturnBadArgument;

    *result = nullptr;
    uint8_t buffer[ArgumentSize] = {};
    if (argumentSize) memcpy(buffer, arguments, argumentSize);

    OSNumber *instance = OSNumber::withNumber(0ULL, 32);
    OSNumber *methodNumber = OSNumber::withNumber(static_cast<unsigned long long>(method), 32);
    OSData *input = OSData::withBytes(buffer, sizeof(buffer));
    IOReturn status = kIOReturnNoMemory;

    if (instance && methodNumber && input) {
        OSObject *parameters[] = {instance, methodNumber, input};
        IOLockLock(lock);
        status = device->evaluateObject("WMNB", result, parameters, 3);
        IOLockUnlock(lock);
    }

    if (instance) instance->release();
    if (methodNumber) methodNumber->release();
    if (input) input->release();
    if (status != kIOReturnSuccess) {
        KernelLog::error("ACPI", "WMNB", "method=0x%08x inputSize=%u status=0x%08x", static_cast<unsigned>(method), static_cast<unsigned>(argumentSize), static_cast<unsigned>(status));
    }
    return status;
}

IOReturn ACPITransport::evaluate(uint32_t method, const uint32_t *arguments, uint32_t argumentCount, uint32_t &result)
{
    if (argumentCount > ArgumentSize / sizeof(uint32_t)) return kIOReturnBadArgument;
    OSObject *object = nullptr;
    IOReturn status = call(method, arguments, argumentCount * sizeof(uint32_t), &object);
    if (status == kIOReturnSuccess) {
        OSNumber *number = OSDynamicCast(OSNumber, object);
        if (number) result = number->unsigned32BitValue();
        else {
            KernelLog::error("ACPI", "evaluate", "method=0x%08x returned a non-integer object", static_cast<unsigned>(method));
            status = kIOReturnBadMedia;
        }
    }
    if (object) object->release();
    return status;
}

IOReturn ACPITransport::evaluateBuffer(uint32_t method, const void *arguments, size_t argumentSize, void *output, size_t &outputSize)
{
    if (!output || outputSize == 0) return kIOReturnBadArgument;
    OSObject *object = nullptr;
    IOReturn status = call(method, arguments, argumentSize, &object);
    if (status == kIOReturnSuccess) {
        OSData *data = OSDynamicCast(OSData, object);
        OSNumber *number = OSDynamicCast(OSNumber, object);
        if (data) {
            size_t required = data->getLength();
            if (outputSize < required) status = kIOReturnNoSpace;
            else memcpy(output, data->getBytesNoCopy(), required);
            outputSize = required;
        } else if (number) {
            uint32_t value = number->unsigned32BitValue();
            if (outputSize < sizeof(value)) status = kIOReturnNoSpace;
            else memcpy(output, &value, sizeof(value));
            outputSize = sizeof(value);
        } else {
            KernelLog::error("ACPI", "evaluateBuffer", "method=0x%08x returned an unsupported object", static_cast<unsigned>(method));
            outputSize = 0;
            status = kIOReturnBadMedia;
        }
    } else {
        outputSize = 0;
    }
    if (object) object->release();
    return status;
}
