//
// Created by moqi on 2019-07-18.
//

#ifndef FRAMEWORK_AF_CONFIG_H
#define FRAMEWORK_AF_CONFIG_H

#ifdef __EMSCRIPTEN__

#ifdef __EMSCRIPTEN_PTHREADS__
#define AF_HAVE_PTHREAD 1
#else
#define AF_HAVE_PTHREAD 0
#endif

#else
#define AF_HAVE_PTHREAD 1
#endif

#ifdef NDEBUG
//#define NOLOGD
#endif


#endif //FRAMEWORK_AF_CONFIG_H
