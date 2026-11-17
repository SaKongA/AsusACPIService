#ifndef ASUS_ACPI_USER_CLIENT_HPP
#define ASUS_ACPI_USER_CLIENT_HPP

#include <IOKit/IOUserClient.h>
#include "../include/AsusACPIProtocol.h"

class AsusACPIService;

class AsusACPIUserClient : public IOUserClient {
    OSDeclareDefaultStructors(AsusACPIUserClient)

public:
    static AsusACPIUserClient *create();
    bool initWithTask(task_t owningTask, void *securityID, UInt32 type, OSDictionary *properties) override;
    bool start(IOService *provider) override;
    void stop(IOService *provider) override;
    IOReturn clientClose() override;
    IOReturn clientDied() override;
    IOReturn externalMethod(uint32_t selector, IOExternalMethodArguments *arguments, IOExternalMethodDispatch *dispatch = nullptr, OSObject *target = nullptr, void *reference = nullptr) override;

private:
    // 向应用返回驱动状态
    static IOReturn getState(OSObject *target, void *reference, IOExternalMethodArguments *arguments);
    // 根据应用请求设置性能模式
    static IOReturn setMode(OSObject *target, void *reference, IOExternalMethodArguments *arguments);
    // 根据应用请求调用分类控制器
    static IOReturn controller(OSObject *target, void *reference, IOExternalMethodArguments *arguments);
    IOReturn readState(AsusACPIState &state);
    IOReturn applyMode(AsusACPIMode mode);
    IOReturn dispatchController(const AsusACPIRequest &request, AsusACPIResponse &response);
    AsusACPIService *service = nullptr;
};

#endif
