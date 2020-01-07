//
// Created by moqi on 2019-08-02.
//

#ifndef FRAMEWORK_AFACTIVEVIDEORENDER_H
#define FRAMEWORK_AFACTIVEVIDEORENDER_H


#include "IVideoRender.h"
#include "render/video/vsync/IVSync.h"

class AFActiveVideoRender : public IVideoRender {

public:
    explicit AFActiveVideoRender(float Hz = 60);

    ~AFActiveVideoRender() override = default;

    virtual void onVSync(int64_t tick) = 0;

    virtual int setHz(float Hz);



protected:
    std::unique_ptr<IVSync> mVSync{};

};


#endif //FRAMEWORK_AFACTIVEVIDEORENDER_H
