//
//  AbrAlgoStrategy.cpp
//  apsara_player
//
//  Created by shiping.csp on 2018/11/1.
//

#include <algorithm>
#include "AbrAlgoStrategy.h"
#include "AbrRefererData.h"

AbrAlgoStrategy::AbrAlgoStrategy(std::function<void(int)> func)
{
    mFunc = func;
    mCurrentBitrate = -1;
}

AbrAlgoStrategy::~AbrAlgoStrategy()
{
    if (mRefererData) {
        delete mRefererData;
    }
}

void AbrAlgoStrategy::Clear()
{
    mBitrates.clear();
    mStreamIndexBitrateMap.clear();
    mCurrentBitrate = -1;
}

void AbrAlgoStrategy::AddStreamInfo(int streamIndex, int bitrate)
{
    mStreamIndexBitrateMap.insert(pair<int, int>(bitrate, streamIndex));
    mBitrates.push_back(bitrate);
    sort(mBitrates.begin(), mBitrates.end(), std::less<int>());
}

void AbrAlgoStrategy::SetCurrentBitrate(int bitrate)
{
    mCurrentBitrate = bitrate;
}

void AbrAlgoStrategy::SetRefererData(AbrRefererData *refererData)
{
    mRefererData = refererData;
}
