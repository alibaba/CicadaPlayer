//
// Created by lifujun on 2020/5/7.
//

#ifndef SOURCE_UTILS_H
#define SOURCE_UTILS_H
#ifdef __cplusplus
extern "C" {
#endif

namespace cicada {

    static inline void convertToGLColor(uint32_t color, float *dst) {
        if(dst == nullptr){
            return;
        }
        
        dst[0] = ((color >> 16) & 0xff) / 255.0f;//r
        dst[1] = ((color >> 8) & 0xff) / 255.0f;//g
        dst[2] = ((color) & 0xff) / 255.0f;//b
        dst[3] = ((color >> 24) & 0xff) / 255.0f;//a
    }
}

#ifdef __cplusplus
}
#endif

#endif //SOURCE_UTILS_H
