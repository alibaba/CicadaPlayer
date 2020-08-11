//
// Created by moqi on 2019-08-05.
//

#ifndef FRAMEWORK_TIMEDVSYNC_H
#define FRAMEWORK_TIMEDVSYNC_H

#include "IVSync.h"
#include <memory>
#include <utils/afThread.h>
#include <utils/af_clock.h>

class timedVSync : public IVSync {
public:
    timedVSync(Listener &listener, float Hz);

    ~timedVSync() override;

    int setHz(float Hz) override;

    void start() override;

    void pause() override;
    float getHz()override{
        return mHz;
    };


private:

    int timedThread();

private:
    std::atomic_int mPeriod; //us
    float mHz{0};
    std::unique_ptr<afThread> mThread{};
    af_clock mClock;
    bool mBPaused{false};

};


#endif //FRAMEWORK_TIMEDVSYNC_H
