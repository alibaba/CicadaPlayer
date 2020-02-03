//
// Created by moqi on 2018/5/11.
//

#ifndef FRAMEWORK_MEDIAFRAME_H
#define FRAMEWORK_MEDIAFRAME_H

#include "AFMediaType.h"
#include "CicadaType.h"

#ifdef __cplusplus
extern "C" {
#endif

CICADA_EXTERN void releaseMeta(Stream_meta *pMeta);

CICADA_EXTERN void releaseSourceMeta(Source_meta *meta);

#ifdef __cplusplus
};
#endif


#endif //FRAMEWORK_MEDIAFRAME_H
