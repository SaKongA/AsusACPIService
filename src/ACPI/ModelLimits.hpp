#ifndef ASUS_ACPI_MODEL_LIMITS_HPP
#define ASUS_ACPI_MODEL_LIMITS_HPP

#include <stdint.h>

struct AsusACPIModelFlags {
    bool advantageEdition = false;
    bool g14AMD = false;
    bool x13 = false;
    bool ally = false;
    bool intelHX = false;
    bool gu606 = false;
    bool dynamicBoost5 = false;
    bool dynamicBoost20 = false;
    bool dynamicBoost15 = false;
    bool cpuLight = false;
    bool z1325 = false;
    bool onlyAIMax = false;
    bool h7607 = false;
};

struct AsusACPIModelLimits {
    uint32_t maxTotal = 150;
    uint32_t defaultTotal = 80;
    uint32_t maxCPU = 100;
    uint32_t maxGPUBoost = 25;
    uint32_t maxCrossLoad = 40;
    uint32_t maxCPUTemperature = 97;
};

AsusACPIModelLimits asusACPIModelLimits(const AsusACPIModelFlags &flags);

#endif
