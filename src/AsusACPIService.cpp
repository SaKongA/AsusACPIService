#include "AsusACPIService.hpp"
#include "AsusACPIUserClient.hpp"
#include "ACPI/ACPITransport.hpp"
#include "Controller/FanController.hpp"
#include "Controller/PerformanceController.hpp"
#include "Diagnostics/KernelLog.hpp"

#define super IOService
OSDefineMetaClassAndStructors(AsusACPIService, IOService)

bool AsusACPIService::start(IOService *provider)
{
    KernelLog::info("Service", "start", "starting driver");
    if (!super::start(provider)) { KernelLog::error("Service", "start", "superclass start failed"); return false; }
    auto *device = OSDynamicCast(IOACPIPlatformDevice, provider);
    if (!device) { KernelLog::error("Service", "start", "provider is not an ACPI platform device"); return false; }
    stateLock = IOLockAlloc();
    if (!stateLock) { KernelLog::error("Service", "start", "failed to allocate state lock"); return false; }
    transport = new ACPITransport;
    if (!transport || !transport->attach(device)) { KernelLog::error("Service", "start", "failed to attach ACPI transport"); return false; }
    acpi = new AsusACPI(*transport);
    if (!acpi) { KernelLog::error("Service", "start", "failed to allocate ACPI interface"); return false; }
    performance = new PerformanceController(*acpi);
    fans = new FanController(*acpi);
    if (!performance || !fans) { KernelLog::error("Service", "start", "failed to allocate controllers"); return false; }
    uint32_t result = 0;
    IOReturn status = acpi->initialize(result);
    if (status != kIOReturnSuccess) { KernelLog::error("Service", "initialize", "ACPI initialization failed status=0x%08x", static_cast<unsigned>(status)); return false; }
    KernelLog::info("Service", "initialize", "ACPI initialization result=0x%08x", static_cast<unsigned>(result));
    active = true;
    registerService();
    KernelLog::info("Service", "start", "driver registered");
    return true;
}

void AsusACPIService::stop(IOService *provider)
{
    KernelLog::info("Service", "stop", "stopping driver");
    if (stateLock) {
        IOLockLock(stateLock);
        active = false;
        IOLockUnlock(stateLock);
    }
    super::stop(provider);
}

void AsusACPIService::free()
{
    delete fans;
    delete performance;
    delete acpi;
    delete transport;
    if (stateLock) IOLockFree(stateLock);
    super::free();
}

IOReturn AsusACPIService::newUserClient(task_t owningTask, void *securityID, UInt32 type, IOUserClient **handler)
{
    if (!handler || type != 0) { KernelLog::warning("Service", "client", "invalid client type=%u", static_cast<unsigned>(type)); return kIOReturnBadArgument; }
    *handler = nullptr;
    auto *client = AsusACPIUserClient::create();
    if (!client) { KernelLog::error("Service", "client", "failed to allocate user client"); return kIOReturnNoMemory; }
    if (!client->initWithTask(owningTask, securityID, type, nullptr)) {
        KernelLog::error("Service", "client", "failed to initialize user client");
        client->release();
        return kIOReturnError;
    }
    if (!client->attach(this)) {
        KernelLog::error("Service", "client", "failed to attach user client");
        client->release();
        return kIOReturnError;
    }
    if (!client->start(this)) {
        KernelLog::error("Service", "client", "failed to start user client");
        client->detach(this);
        client->release();
        return kIOReturnError;
    }
    *handler = client;
    KernelLog::info("Service", "client", "user client connected");
    return kIOReturnSuccess;
}

