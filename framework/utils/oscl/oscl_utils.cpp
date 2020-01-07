
#if __APPLE__
    #include <TargetConditionals.h>
#endif
#if TARGET_OS_IPHONE
    #include "ios_utils.h"
#endif
#ifdef ANDROID
    #include "linux_utils.h"
#endif
#include <stdio.h>
#include "oscl_utils.h"

#include <mutex>

static std::mutex gMutex;

int AFGetSystemMemInfo(mem_info *pInfo)
{
    // int ret;
    if (pInfo == NULL)
        return -1;

#if TARGET_OS_IPHONE || defined ANDROID
    std::lock_guard<std::mutex> lock(gMutex);
    return get_system_meminfo(pInfo);
#endif
    return -1;
}

int64_t AFGetSysLowMem(int level)
{
#ifdef ANDROID
    std::lock_guard<std::mutex> lock(gMutex);
    return android_get_low_mem(level);
#endif
    return -1;
}
