//
//  AbrBufferRefererData.h
//  apsara_player
//
//  Created by shiping.csp on 2018/11/1.
//

#ifndef AbrBufferRefererData_h
#define AbrBufferRefererData_h

#include "AbrRefererData.h"
#include <cstdio>

class AbrBufferRefererData : public AbrRefererData {
public:
    explicit AbrBufferRefererData(void *playerPtr);
    ~AbrBufferRefererData() override;

public:
    //measure current packet length
    int64_t GetCurrentPacketBufferLength() override;

    //get max buffer duration
    int64_t GetMaxBufferDurationInConfig() override;

    int GetRemainSegmentCount() override;

    bool GetIsConnected() override;

    bool GetReBuffering() override;

    int64_t GetCurrentDownloadSpeed() override;

    void setCurrentDownloadSpeed(int64_t speed);

    bool IsDownloadCompleted() override;


private:
    void *mHandle = nullptr;
    int64_t mCurrentDownloadSpeed{0};
};

#endif /* AbrBufferRefererData_h */
