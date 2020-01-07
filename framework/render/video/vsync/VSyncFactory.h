//
// Created by lifujun on 2019/9/6.
//

#ifndef SOURCE_VSYNCFACTROY_H
#define SOURCE_VSYNCFACTROY_H


#include <memory>
#include "IVSync.h"

class VSyncFactory {
public:
    static std::unique_ptr<IVSync> create(IVSync::Listener &listener, float HZ);
};


#endif //SOURCE_VSYNCFACTROY_H
