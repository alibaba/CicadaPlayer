//
//  AbrRefererData.h
//  apsara_player
//
//  Created by shiping.csp on 2018/11/1.
//

#ifndef AbrRefererData_h
#define AbrRefererData_h

#include <stdio.h>
#include <cstdint>


class AbrRefererData
{
public:
    AbrRefererData();
    virtual ~AbrRefererData();

public:

    //measure current packet length
    virtual int64_t GetCurrentPacketBufferLength() = 0;

    //get max buffer duration
    virtual int64_t GetMaxBufferDurationInConfig() = 0;

    virtual int GetRemainSegmentCount() = 0;

    virtual bool GetIsConnected() {return true;}

    //measure network strength
    virtual int64_t GetDownloadedBytes();
};

#endif /* AbrRefererData_h */
