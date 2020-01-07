#ifndef __OSCL_UTILS_H__
#define __OSCL_UTILS_H__

#include <stdint.h>

typedef struct {
    uint64_t system_totalram;
    uint64_t system_availableram;
    uint64_t system_freeram;
    uint64_t self_useram;
} mem_info;

#ifdef __cplusplus
extern "C" {
#endif

int AFGetSystemMemInfo(mem_info *pInfo);

int64_t AFGetSysLowMem(int level);

#ifdef __cplusplus
};
#endif

#endif