#ifndef ASUS_ACPI_PROTOCOL_H
#define ASUS_ACPI_PROTOCOL_H

#include <stdint.h>

enum {
    AsusACPIProtocolVersion = 1,
    AsusACPIValidLastSetMode = 1u << 0,
    AsusACPIValidCPUFan = 1u << 1,
    AsusACPIValidGPUFan = 1u << 2,
    AsusACPICapabilitySetMode = 1u << 0,
    AsusACPICapabilityCPUFan = 1u << 1,
    AsusACPICapabilityGPUFan = 1u << 2
};

typedef enum {
    AsusACPIModeBalanced = 0,
    AsusACPIModePerformance = 1,
    AsusACPIModeSilent = 2
} AsusACPIMode;

typedef enum {
    // 读取驱动状态与风扇转速
    AsusACPICommandGetState = 0,
    // 设置静音、平衡或性能模式
    AsusACPICommandSetMode = 1,
    // 调用分类控制器接口
    AsusACPICommandController = 2
} AsusACPICommand;

typedef enum {
    // 读取性能模式到 values[0] 并返回固件原值到 values[1]
    AsusACPIOpPerformanceReadMode,
    // 使用 arguments[0] 设置性能模式并返回固件结果
    AsusACPIOpPerformanceSetMode,
    // 使用 arguments[0] 设置性能状态模式并返回固件结果
    AsusACPIOpPerformanceSetStatusMode,
    // 使用 payload 中的 AsusACPIPerformanceSettings 应用性能配置并返回是否使用风扇范围回退
    AsusACPIOpPerformanceApply,
    // 使用 arguments[0] 指定风扇并读取 RPM
    AsusACPIOpFanReadSpeed,
    // 使用 arguments[0] 指定风扇并读取对应温度
    AsusACPIOpFanReadTemperature,
    // 使用 arguments[0] 指定风扇和 arguments[1] 指定模式并将曲线返回到 payload
    AsusACPIOpFanReadCurve,
    // 使用 arguments[0] 指定风扇并将最低转速曲线返回到 payload
    AsusACPIOpFanReadFloor,
    // 将风扇升温和降温迟滞参数返回到 values[0] 和 values[1]
    AsusACPIOpFanReadHysteresis,
    // 使用 arguments[0...2] 设置升温迟滞、降温迟滞和是否包含中间风扇
    AsusACPIOpFanSetHysteresis,
    // 使用 arguments[0] 指定风扇、arguments[1] 指定比例并通过 payload 传入 16 字节曲线
    AsusACPIOpFanSetCurve,
    // 使用 arguments[0] 指定风扇并通过 payload 传入 16 字节曲线
    AsusACPIOpFanSetRange,
    // 使用 arguments[0] 指定是否限制曲线点并通过 payload 传入和返回 16 字节曲线
    AsusACPIOpFanFixCurve,
    // 使用 arguments[0...1] 指定比例和回退模式并在 payload 依次传入 CPU、GPU 和可选中间风扇曲线
    AsusACPIOpFanApplyCurves,
    // 使用 arguments[0] 指定 AsusACPIPowerFeature 并返回参数
    AsusACPIOpPowerRead,
    // 使用 arguments[0...1] 指定 AsusACPIPowerFeature 和写入值
    AsusACPIOpPowerWrite,
    // 将电池放电功率以有符号百分之一瓦返回到 values[0]
    AsusACPIOpPowerReadBatteryDischarge,
    // 使用 payload 中的 AsusACPIPowerSettings 应用功耗配置
    AsusACPIOpPowerApply,
    // 使用 arguments[0] 指定 AsusACPIGraphicsFeature 并返回参数
    AsusACPIOpGraphicsRead,
    // 使用 arguments[0...1] 指定 AsusACPIGraphicsFeature 和写入值
    AsusACPIOpGraphicsWrite,
    // 读取 APU 显存档位
    AsusACPIOpGraphicsReadApuMemory,
    // 读取显存选项
    AsusACPIOpGraphicsReadVramOptions,
    // 读取显存配置
    AsusACPIOpGraphicsReadVramMemory,
    // 使用 arguments[0] 设置 APU 显存档位
    AsusACPIOpGraphicsSetApuMemory,
    // 读取外接显示器连接状态
    AsusACPIOpGraphicsExternalDisplay,
    // 读取外接显卡连接状态
    AsusACPIOpGraphicsExternalGPU,
    // 使用 arguments[0...1] 指定 Eco 和 MUX 设备并返回 AsusACPIGraphicsMode
    AsusACPIOpGraphicsReadMode,
    // 使用 arguments[0...2] 指定 Eco 设备、MUX 设备和模式并返回是否需要重启
    AsusACPIOpGraphicsSetMode,
    // 使用 arguments[0] 指定是否仅有集显并返回全 AMD 功耗接口支持情况
    AsusACPIOpGraphicsIsAllAmdPpt,
    // 使用 arguments[0...2] 指定 Eco 设备、是否仅有集显和是否为 Ally 并返回 NVIDIA 接口支持情况
    AsusACPIOpGraphicsIsNvidiaGPU,
    // 使用 arguments[0] 指定 AsusACPIDisplayFeature 并返回参数
    AsusACPIOpDisplayRead,
    // 使用 arguments[0...1] 指定 AsusACPIDisplayFeature 和写入值
    AsusACPIOpDisplayWrite,
    // 检查屏幕超频响应支持情况
    AsusACPIOpDisplaySupportsOverdrive,
    // 使用 arguments[0] 设置 Mini LED 模式
    AsusACPIOpDisplaySetMiniLed,
    // 使用 arguments[0] 以有符号值设置 ScreenPad 亮度百分比或传入 -1 关闭
    AsusACPIOpDisplaySetScreenPadBrightness,
    // 读取 Fn 锁状态
    AsusACPIOpKeyboardReadFnLock,
    // 使用 arguments[0] 设置 Fn 锁开启状态
    AsusACPIOpKeyboardSetFnLock,
    // 使用 arguments[0] 设置键盘灯亮度
    AsusACPIOpKeyboardSetBrightness,
    // 使用 arguments[0...4] 设置灯效模式、红色、绿色、蓝色和速度
    AsusACPIOpKeyboardSetRGB,
    // 使用 arguments[0...4] 设置运行、启动、睡眠、关机和 Vivo 机型标志
    AsusACPIOpKeyboardSetPower,
    // 使用 arguments[0] 发送 AsusACPIFirmwareKey 固件快捷键事件
    AsusACPIOpKeyboardSendFirmwareKey,
    // 使用 arguments[0] 指定 AsusACPIDeviceFeature 并返回参数
    AsusACPIOpDeviceRead,
    // 使用 arguments[0...1] 指定 AsusACPIDeviceFeature 和写入值
    AsusACPIOpDeviceWrite,
    // 使用 arguments[0] 指定 CPU 核心设备并返回能效核和性能核数量
    AsusACPIOpDeviceReadCores,
    // 使用 arguments[0...1] 设置能效核和性能核数量
    AsusACPIOpDeviceSetCores,
    // 切换摄像头遮挡状态并返回摄像头开启状态和固件结果
    AsusACPIOpDeviceToggleCameraShutter,
    // 使用 arguments[0] 设置状态指示灯开启状态
    AsusACPIOpDeviceSetStatusLed,
    // 读取 VoodooI2C 触控板启用状态
    AsusACPIOpTouchpadRead,
    // 使用 arguments[0] 设置 VoodooI2C 触控板启用状态
    AsusACPIOpTouchpadSet
} AsusACPIOperation;

typedef enum {
    AsusACPIFanCPU = 0,
    AsusACPIFanGPU = 1,
    AsusACPIFanMiddle = 2
} AsusACPIFan;

typedef enum {
    AsusACPIGraphicsModeEco = 0,
    AsusACPIGraphicsModeStandard = 1,
    AsusACPIGraphicsModeUltimate = 2
} AsusACPIGraphicsMode;

typedef enum {
    AsusACPIPowerCharger = 0x0012006C,
    AsusACPIPowerBatteryLimit = 0x00120057,
    AsusACPIPowerApuSlowBoost = 0x001200A0,
    AsusACPIPowerCpuEdc = 0x001200A1,
    AsusACPIPowerCpuTdc = 0x001200A2,
    AsusACPIPowerApuSustained = 0x001200A3,
    AsusACPIPowerCpuPpt = 0x001200B0,
    AsusACPIPowerTotalPpt = 0x001200B1,
    AsusACPIPowerCrossLoadLegacy = 0x0012009A,
    AsusACPIPowerGpuToCpu = 0x0012009C,
    AsusACPIPowerCpuTemperatureLimit = 0x0012009E,
    AsusACPIPowerCrossLoad = 0x0012009F,
    AsusACPIPowerGpuBoost = 0x001200C0,
    AsusACPIPowerApuFastBoost = 0x001200C1,
    AsusACPIPowerGpuTemperatureTarget = 0x001200C2,
    AsusACPIPowerGpuBase = 0x00120099,
    AsusACPIPowerGpuAdditional = 0x00120098
} AsusACPIPowerFeature;

typedef enum {
    AsusACPIGraphicsRogEco = 0x00090020,
    AsusACPIGraphicsVivoEco = 0x00090120,
    AsusACPIGraphicsExternalGpuConnected = 0x00090018,
    AsusACPIGraphicsExternalGpu = 0x00090019,
    AsusACPIGraphicsExternalDisplay = 0x00090030,
    AsusACPIGraphicsRogMux = 0x00090016,
    AsusACPIGraphicsVivoMux = 0x00090026,
    AsusACPIGraphicsApuMemory = 0x000600C1,
    AsusACPIGraphicsVramMemory = 0x000600C4
} AsusACPIGraphicsFeature;

typedef enum {
    AsusACPIDisplayOverdrive = 0x00050019,
    AsusACPIDisplayOverdriveSupport = 0x00050020,
    AsusACPIDisplayMiniLed = 0x0005001E,
    AsusACPIDisplayMiniLedAlternative = 0x0005002E,
    AsusACPIDisplayFullHD = 0x0005001C,
    AsusACPIDisplayHDR = 0x00050071,
    AsusACPIDisplayOptimalBrightness = 0x0005002A,
    AsusACPIDisplayInitialize = 0x00050011,
    AsusACPIDisplayScreenPadToggle = 0x00050031,
    AsusACPIDisplayScreenPadBrightness = 0x00050032
} AsusACPIDisplayFeature;

typedef enum {
    AsusACPIDeviceMicrophoneMuteLed = 0x00040017,
    AsusACPIDeviceSoundMuteLed = 0x0004001C,
    AsusACPIDeviceSlateMode = 0x00120063,
    AsusACPIDeviceTabletState = 0x00060077,
    AsusACPIDeviceTentState = 0x00060062,
    AsusACPIDeviceCameraShutter = 0x00060078,
    AsusACPIDeviceCameraLed = 0x00060079,
    AsusACPIDeviceStatusLed = 0x000600C2,
    AsusACPIDeviceBootSound = 0x00130022,
    AsusACPIDeviceCpuCores = 0x001200D2,
    AsusACPIDeviceCpuCoresMax = 0x001200D3,
    AsusACPIDeviceCpuCoresMin = 0x001200D4
} AsusACPIDeviceFeature;

typedef enum {
    AsusACPIFirmwareKeyBrightnessDown = 0x10,
    AsusACPIFirmwareKeyBrightnessUp = 0x20,
    AsusACPIFirmwareKeySleep = 0x6C,
    AsusACPIFirmwareKeyAirplane = 0x88
} AsusACPIFirmwareKey;

typedef struct {
    int32_t total, cpu, slow, fast, gpuToCpu, crossLoad, cpuTemperature;
    int32_t gpuBoost, gpuTemperature, gpuAdditional;
    uint32_t amdCPU, integratedOnly;
    uint32_t maxTotal, maxCPU, maxGPUBoost, maxCrossLoad, maxCPUTemperature;
} AsusACPIPowerSettings;

typedef struct {
    uint32_t mode, applyStatusMode, manualMode, fanScale;
    uint32_t hasFans, hasMiddleFan, hysteresisUp, hysteresisDown, hasPower;
    uint8_t cpuCurve[16], gpuCurve[16], middleCurve[16];
    AsusACPIPowerSettings power;
} AsusACPIPerformanceSettings;

typedef struct {
    uint32_t operation;
    uint32_t arguments[8];
    uint32_t payloadSize;
    uint8_t payload[160];
} AsusACPIRequest;

typedef struct {
    uint32_t values[8];
    uint32_t payloadSize;
    uint8_t payload[160];
} AsusACPIResponse;

typedef struct {
    uint32_t version;
    uint32_t capabilities;
    uint32_t lastSetMode;
    uint32_t cpuFanRPM;
    uint32_t gpuFanRPM;
    uint32_t validFields;
} AsusACPIState;

#endif
