//
// Created by moqi on 2018/4/25.
//

#ifndef FRAMEWORK_SEGMENT_H
#define FRAMEWORK_SEGMENT_H

#include "SegmentPart.h"
#include "segment_decrypt/SegmentEncryption.h"
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>

//using namespace std;
namespace Cicada{

    enum SegTypes
    {
        SEG_NORMAL = 0,
        SEG_LHLS,
    };

    class segment {
    public:
        explicit segment(uint64_t seq);

        virtual ~segment();

        uint64_t getSequenceNumber();

        void setSequenceNumber(uint64_t seq);

        void setSourceUrl(const std::string &value);

        void print();

        void setEncryption(const std::vector<SegmentEncryption> &enc);

        void setByteRange(int64_t start, int64_t end);

        // Low-Latency HLS
        std::string getDownloadUrl();
        void updateParts(const std::vector<SegmentPart> &parts);
        const std::vector<SegmentPart> &getSegmentParts();
        void moveToNextPart();
        void moveToPart(int partIndex);
        void moveToNearestIndependentPart(int partIndex);
        bool isDownloadComplete(bool &bHasUnusedParts);
        
    public:
        std::string mUri = "";
        uint64_t startTime = 0;
        int64_t duration = -1;
        uint64_t sequence = 0;
        uint64_t discontinuityNum = 0;
        static const int SEQUENCE_INVALID;
        static const int SEQUENCE_FIRST;
        std::vector<SegmentEncryption> encryptions;
        int64_t rangeStart {INT64_MIN};
        int64_t rangeEnd {INT64_MIN};

        std::shared_ptr<segment> init_section{nullptr};

        // Low-Latency HLS
        SegTypes mSegType = SEG_NORMAL;
        std::vector<SegmentPart> mParts;
        int mPartsNextIndex = 0;
        std::recursive_mutex mMutex;
        std::string mDownloadUri = "";
    };
}


#endif //FRAMEWORK_SEGMENT_H
