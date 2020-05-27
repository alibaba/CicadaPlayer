//
//  CicadaType.h
//
//  Created by huang_jiafa on 2019/3/6.
//  Copyright (c) 2019 Aliyun. All rights reserved.
//

#ifndef Cicada_H
#define Cicada_H

#if defined(_MSC_VER)
  #define attribute_deprecated __declspec(deprecated)
  #define CICADA_CPLUS_EXTERN __declspec(dllexport)
#elif defined _WIN32 || defined __CYGWIN__
  #define attribute_deprecated __attribute__((deprecated))
  #ifdef __GNUC__
    #define CICADA_CPLUS_EXTERN __attribute__ ((dllexport))
  #else
    #define CICADA_CPLUS_EXTERN __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
  #endif
#else
  #if __GNUC__ >= 4
    #define CICADA_CPLUS_EXTERN __attribute__((visibility ("default")))
    #define attribute_deprecated __attribute__((deprecated))
  #else
    #define CICADA_CPLUS_EXTERN
    #define attribute_deprecated
  #endif
#endif

#ifdef __GNUC__
#    define AV_GCC_VERSION_AT_LEAST(x,y) (__GNUC__ > (x) || __GNUC__ == (x) && __GNUC_MINOR__ >= (y))
#    define AV_GCC_VERSION_AT_MOST(x,y)  (__GNUC__ < (x) || __GNUC__ == (x) && __GNUC_MINOR__ <= (y))
#else
#    define AV_GCC_VERSION_AT_LEAST(x,y) 0
#    define AV_GCC_VERSION_AT_MOST(x,y)  0
#endif

#if AV_GCC_VERSION_AT_LEAST(3,4)
#    define attribute_warn_unused_result __attribute__((warn_unused_result))
#else
#    define attribute_warn_unused_result
#endif

#define CICADA_EXTERN CICADA_CPLUS_EXTERN

#endif //Cicada_H


