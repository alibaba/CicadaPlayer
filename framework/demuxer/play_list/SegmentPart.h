//
//  SegmentPart.h
//  CicadaPlayerSDK
//
//  Created by weixin on 2020/9/21.
//

#ifndef SegmentPart_h
#define SegmentPart_h

#include <string>
using namespace std;

namespace Cicada
{
    typedef struct SegmentPart
    {
        int64_t duration;
        string uri;
        bool independent;
        uint64_t sequence;
        
        SegmentPart()
        {
            duration = 0;
            uri = "";
            independent = false;
            sequence = 0;
        }
    } SegmentPart;
}

#endif /* SegmentPart_h */
