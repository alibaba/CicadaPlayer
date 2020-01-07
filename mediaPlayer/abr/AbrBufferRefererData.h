//
//  AbrBufferRefererData.h
//  apsara_player
//
//  Created by shiping.csp on 2018/11/1.
//

#ifndef AbrBufferRefererData_h
#define AbrBufferRefererData_h

#include <stdio.h>
#include "AbrRefererData.h"

class AbrBufferRefererData : public AbrRefererData
{
public:
    AbrBufferRefererData(void* playerPtr);
    virtual ~AbrBufferRefererData();
    
public:
    //measure current packet length
    virtual int64_t GetCurrentPacketBufferLength();
    
    //get max buffer duration
    virtual int64_t GetMaxBufferDurationInConfig();

    virtual int GetRemainSegmentCount();

    virtual bool GetIsConnected();

private:
    void* mHandle = nullptr;
};

#endif /* AbrBufferRefererData_h */
