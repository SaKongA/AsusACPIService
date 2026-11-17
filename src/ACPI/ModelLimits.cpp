#include "ModelLimits.hpp"

AsusACPIModelLimits asusACPIModelLimits(const AsusACPIModelFlags &flags)
{
    AsusACPIModelLimits limits;
    if (flags.advantageEdition) limits.maxTotal = 250;
    if (flags.g14AMD) limits.defaultTotal = 125;
    if (flags.x13) { limits.maxTotal = 75; limits.defaultTotal = 50; }
    if (flags.ally) { limits.maxTotal = 50; limits.defaultTotal = 30; }
    if (flags.intelHX) { limits.maxTotal = 175; limits.maxCrossLoad = 125; limits.maxCPUTemperature = 103; }
    if (flags.gu606) limits.maxCrossLoad = 50;
    if (flags.dynamicBoost5) limits.maxGPUBoost = 5;
    if (flags.dynamicBoost20) limits.maxGPUBoost = 20;
    if (flags.dynamicBoost15) limits.maxGPUBoost = 15;
    if (flags.cpuLight) limits.maxTotal = 90;
    if (flags.z1325) limits.maxTotal = 93;
    if (flags.onlyAIMax) { limits.maxTotal = 115; limits.maxCPU = 115; }
    if (flags.h7607) limits.maxTotal = 95;
    return limits;
}
