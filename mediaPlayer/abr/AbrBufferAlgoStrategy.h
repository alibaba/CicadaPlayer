//
//  AbrBufferAlgoStrategy.h
//  apsara_player
//
//  Created by shiping.csp on 2018/11/1.
//

#ifndef AbrBufferAlgoStrategy_h
#define AbrBufferAlgoStrategy_h

#include "AbrAlgoStrategy.h"
#include <list>

using namespace std;

class AbrBufferAlgoStrategy:public AbrAlgoStrategy
{
public:
    AbrBufferAlgoStrategy(std::function<void(int)> func);
    ~AbrBufferAlgoStrategy();
    
public:
    void Reset() override;
    
    void ProcessAbrAlgo() override;

    void SetCurrentBitrate(int bitrate) override;

protected:
    void ComputeBufferTrend(int64_t curTime);
    void SwitchBitrate(bool up, int64_t speed, int64_t maxSpeed);
    
protected:
    bool mSwitching = false;
    int mUpSpan = 10*1000;
    int64_t mLastSwitchTimeMS = INT64_MIN;
    int64_t mLastBufferDuration = INT64_MIN;
    int64_t mLastDownloadBytes = 0;

    std::list<bool> mIsUpHistory;
    std::list<int> mBufferStatics;
    std::list<int64_t> mDownloadSpeed;
};

#endif /* AbrBufferAlgoStrategy_h */
