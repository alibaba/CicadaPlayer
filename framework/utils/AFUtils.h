//
// Created by moqi on 2019-05-23.
//

#ifndef SOURCE_AFUTILS_H
#define SOURCE_AFUTILS_H

#include <signal.h>

#ifdef __cplusplus
extern "C"{
#endif

int AFGetCpuCount();

void ignore_signal(int sig);

#ifdef __cplusplus
};
#endif

#endif //SOURCE_AFUTILS_H
