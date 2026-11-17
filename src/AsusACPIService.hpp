#ifndef ASUS_ACPI_SERVICE_HPP
#define ASUS_ACPI_SERVICE_HPP

#include <IOKit/IOService.h>
#include <IOKit/IOLocks.h>
#include "../include/AsusACPIProtocol.h"

class ACPITransport;
class AsusACPI;
class PerformanceController;
class FanController;

class AsusACPIService : public IOService {
    OSDeclareDefaultStructors(AsusACPIService)

public:
    bool start(IOService *provider) override;
    void stop(IOService *provider) override;
    void free() override;
    IOReturn newUserClient(task_t owningTask, void *securityID, UInt32 type, IOUserClient **handler) override;

private:
    friend class AsusACPIUserClient;
    ACPITransport *transport = nullptr;
    AsusACPI *acpi = nullptr;
    PerformanceController *performance = nullptr;
    FanController *fans = nullptr;
    IOLock *stateLock = nullptr;
    bool active = false;
    bool hasSetMode = false;
    AsusACPIMode lastSetMode = AsusACPIModeBalanced;
};

#endif
