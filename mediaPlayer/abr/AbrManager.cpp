//
//  AbrManager.cpp
//  apsara_player
//
//  Created by shiping.csp on 2018/11/1.
//

#define LOG_TAG "AbrManager"

#include "AbrManager.h"
#include "utils/afThread.h"
#include "AbrAlgoStrategy.h"
#include "utils/timer.h"

AbrManager::AbrManager()
{
    mPMainThread = NEW_AF_THREAD(AbrAdjustFun);
    mMsgProcessTime = 1000;
}

AbrManager::~AbrManager()
{
    Stop();
    delete mPMainThread;
}

void AbrManager::Reset()
{
    std::unique_lock<std::mutex> uMutex(mMutex);

    if (mAlgoStrategy) {
        mAlgoStrategy->Reset();
    }
}

void AbrManager::Start()
{
    {
        std::unique_lock<std::mutex> uMutex(mMutex);
        mRunning = true;
    }
    mPMainThread->start();
}

void AbrManager::Pause()
{
    {
        std::unique_lock<std::mutex> uMutex(mMutex);
        mRunning = false;
    }
    mCondition.notify_one();
    mPMainThread->pause();
}

void AbrManager::Stop()
{
    {
        std::unique_lock<std::mutex> uMutex(mMutex);
        mRunning = false;
    }
    mCondition.notify_one();
    mPMainThread->stop();
}

void AbrManager::EnableAbr(bool bEnabled)
{
    mEnableAbr = bEnabled;
}

void AbrManager::SetAbrAlgoStrategy(AbrAlgoStrategy *abrAlgo)
{
    mAlgoStrategy = abrAlgo;
}

int AbrManager::AbrAdjustFun()
{
    std::unique_lock<std::mutex> uMutex(mMutex);
    mCondition.wait_for(uMutex, std::chrono::milliseconds(mMsgProcessTime), [this]() { return !mRunning; });

    if (mAlgoStrategy && mEnableAbr) {
        mAlgoStrategy->ProcessAbrAlgo();
    }

    return 0;
}
