//
//  AbrAlgoStrategy.cpp
//  apsara_player
//
//  Created by shiping.csp on 2018/11/1.
//

#include "AbrAlgoStrategy.h"
#include "AbrRefererData.h"
#include <algorithm>
#include <utility>

AbrAlgoStrategy::AbrAlgoStrategy(std::function<void(int)> func)
{
    mFunc = std::move(func);
    mCurrentBitrate = -1;
}

AbrAlgoStrategy::~AbrAlgoStrategy()
{
    delete mRefererData;
}

void AbrAlgoStrategy::Clear()
{
    mBitRates.clear();
    mStreamIndexBitrateMap.clear();
    mCurrentBitrate = -1;
}

void AbrAlgoStrategy::AddStreamInfo(int streamIndex, int bitrate)
{
    mStreamIndexBitrateMap.insert(pair<int, int>(bitrate, streamIndex));
    mBitRates.push_back(bitrate);
    sort(mBitRates.begin(), mBitRates.end(), std::less<int>());
}

void AbrAlgoStrategy::SetCurrentBitrate(int bitrate)
{
    mCurrentBitrate = bitrate;
}

void AbrAlgoStrategy::SetRefererData(AbrRefererData *refererData)
{
    mRefererData = refererData;
}
