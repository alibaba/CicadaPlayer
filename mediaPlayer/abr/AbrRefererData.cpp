//
//  AbrRefererData.cpp
//  apsara_player
//
//  Created by shiping.csp on 2018/11/1.
//

#include "AbrRefererData.h"
#include <string.h>
#include <utils/frame_work_log.h>

//ios and mac
#ifdef __APPLE__
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <net/if.h>
#else
#ifdef WIN32
//todo: win32
#else
//android
#include "TrafficStats.h"
#endif
#endif

AbrRefererData::AbrRefererData()
{
    
}

AbrRefererData::~AbrRefererData()
{
    
}

int64_t AbrRefererData::GetDownloadedBytes()
{
    //ios and mac
#ifdef __APPLE__
    struct ifaddrs *ifa_list = 0, *ifa;
    if (getifaddrs(&ifa_list) == -1)
    {
        return 0;
    }
    
    uint32_t iBytes = 0;
    uint32_t oBytes = 0;
    for (ifa = ifa_list; ifa; ifa = ifa->ifa_next)
    {
        if (AF_LINK != ifa->ifa_addr->sa_family)
            continue;
        
        if (!(ifa->ifa_flags & IFF_UP) && !(ifa->ifa_flags & IFF_RUNNING))
            continue;
        
        if (ifa->ifa_data == 0)
            continue;
        
        /* Not a loopback device. */
        if (strncmp(ifa->ifa_name, "lo", 2))
        {
            struct if_data *if_data = (struct if_data *)ifa->ifa_data;
            iBytes += if_data->ifi_ibytes;
            oBytes += if_data->ifi_obytes;
        }
    }
    
    freeifaddrs(ifa_list);
    
    return iBytes;
#else
    
#ifdef WIN32
    //todo:win32 get downloaed bytes
#elif defined(ANDROID)
    //android
    uint64_t  byteNums = getIfaceStatType(NULL, IfaceStatType::RX_BYTES);
    AF_LOGD("android get bytes .. %lld", byteNums);
    return byteNums;
#endif
    
#endif
    
    return 0;
}
