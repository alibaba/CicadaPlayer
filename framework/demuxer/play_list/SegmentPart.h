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

namespace Cicada {
    typedef struct SegmentPart {
        bool independent{false};
        int64_t duration{0};
        uint64_t sequence{0};
        int64_t rangeStart{INT64_MIN};
        int64_t rangeEnd{INT64_MIN};
        std::string uri;
    } SegmentPart;

    struct PreloadHint {
        bool isPartialSegment{false};
        bool used{false};
        int64_t rangeStart{INT64_MIN};
        int64_t rangeEnd{INT64_MIN};
        std::string uri;
    };
}// namespace Cicada

#endif /* SegmentPart_h */
