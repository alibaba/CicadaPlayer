//
//  AbrAlgoStrategy.h
//  apsara_player
//
//  Created by shiping.csp on 2018/11/1.
//

#ifndef AbrAlgoStrategy_h
#define AbrAlgoStrategy_h

#include <cstdint>
#include <cstdio>
#include <functional>
#include <map>
#include <vector>

using namespace std;

class AbrRefererData;

class AbrAlgoStrategy {
public:
    explicit AbrAlgoStrategy(std::function<void(int)> func);

    virtual ~AbrAlgoStrategy();

public:
    //set referer data source
    virtual void SetRefererData(AbrRefererData *refererData);

    //add stream index and bitrate info
    virtual void AddStreamInfo(int streamIndex, int bitrate);

    //set current stream
    virtual void SetCurrentBitrate(int bitrate);

    virtual void Clear();
    virtual void SetDuration(int64_t ms) {mDurationMS = ms;}

    //reset abr algo
    virtual void Reset() = 0;

    virtual void ProcessAbrAlgo() = 0;

    virtual void GetOption(const std::string &key, std::string &value)
    {}

    virtual uint32_t GetBitRateCount()
    {
        return mBitRates.size();
    }

protected:
    AbrRefererData *mRefererData = nullptr;
    map<int, int> mStreamIndexBitrateMap;
    vector<int> mBitRates;
    int mCurrentBitrate = 0;
    int mPreBitrate = 0;
    int64_t mDurationMS = -1;
    std::function<void(int)> mFunc;
    void *mUserData = nullptr;
};

#endif /* AbrAlgoStrategy_h */
