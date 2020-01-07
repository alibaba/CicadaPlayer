//
// Created by moqi on 2018/5/12.
//

#ifndef FRAMEWORK_TIMER_H
#define FRAMEWORK_TIMER_H

#include <stdint.h>
#include "CicadaType.h"

#ifdef __cplusplus
extern "C" {
#endif

    CICADA_EXTERN int64_t af_gettime_ms();

    CICADA_EXTERN int64_t af_getsteady_ms();

    CICADA_EXTERN int64_t af_gettime();

    CICADA_EXTERN int64_t af_gettime_relative();

    CICADA_EXTERN void af_msleep(int ms);

    CICADA_EXTERN void af_usleep(int us);

    CICADA_EXTERN int af_make_abstime_latems(struct timespec *pAbstime, uint32_t ms);

#ifdef __cplusplus
};
#endif

#endif //FRAMEWORK_TIMER_H
