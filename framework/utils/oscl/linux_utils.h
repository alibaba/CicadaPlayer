#ifndef __LINUX_UTILS_H__
#define __LINUX_UTILS_H__

#include "oscl_utils.h"

#ifdef __cplusplus
extern "C"{
#endif

int get_system_meminfo(mem_info *pInfo);

#ifdef ANDROID

int64_t android_get_low_mem(int level);

#ifdef __cplusplus
};
#endif

#endif
#endif