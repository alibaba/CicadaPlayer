//
//  AbrManager.h
//  apsara_player
//
//  Created by shiping.csp on 2018/11/1.
//

#ifndef AbrManager_h
#define AbrManager_h

#include <stdio.h>
#include <mutex>
#include <condition_variable>
#include <atomic>

class afThread;

class AbrAlgoStrategy;

class AbrManager {
public:
    AbrManager();

    ~AbrManager();

public:
    //reset abr
    void Reset();

    void Start();

    void Pause();

    void Stop();

    //enable abr or not
    void EnableAbr(bool bEnabled);

    bool IsEnableAbr()
    { return mEnableAbr; }

    //set Abr Aglo strategy
    void SetAbrAlgoStrategy(AbrAlgoStrategy *abrAlgo);

protected:
    int AbrAdjustFun();

protected:
    afThread *mPMainThread = nullptr;
    std::atomic_bool mEnableAbr{false};
    int mMsgProcessTime;
    AbrAlgoStrategy *mAlgoStrategy = nullptr;
    std::atomic_bool mRunning{true};
    std::mutex mMutex;
    std::condition_variable mCondition;
};

#endif /* AbrManager_h */
