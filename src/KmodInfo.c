#include <mach/mach_types.h>

static kern_return_t AsusACPIServiceStart(kmod_info_t *info, void *data)
{
    (void)info;
    (void)data;
    return KERN_SUCCESS;
}

static kern_return_t AsusACPIServiceStop(kmod_info_t *info, void *data)
{
    (void)info;
    (void)data;
    return KERN_SUCCESS;
}

__attribute__((visibility("default")))
KMOD_EXPLICIT_DECL(com.kayson.AsusACPIService, "0.1.0", AsusACPIServiceStart, AsusACPIServiceStop)

__private_extern__ kmod_start_func_t *_realmain = 0;
__private_extern__ kmod_stop_func_t *_antimain = 0;
__private_extern__ int _kext_apple_cc = __APPLE_CC__;
