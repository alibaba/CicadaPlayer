//
//  SegmentPart.h
//  CicadaPlayerSDK
//
//  Created by weixin on 2020/9/21.
//

#ifndef SegmentPart_h
#define SegmentPart_h

#include <string>

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

    struct RenditionReport {
        std::string uri;
        int64_t lastMsn{-1};
        int64_t lastPart{-1};
    };
}// namespace Cicada

#endif /* SegmentPart_h */
