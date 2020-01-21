//
// Created by moqi on 2018/4/25.
//

#ifndef FRAMEWORK_SEGMENT_H
#define FRAMEWORK_SEGMENT_H


#include <cstdint>
#include <string>
#include <memory>
#include "demuxer/play_list/segment_decrypt/SegmentEncryption.h"

//using namespace std;
namespace Cicada{
    class segment {
    public:
        explicit segment(uint64_t seq);

        ~segment();

        uint64_t getSequenceNumber();

        void setSequenceNumber(uint64_t seq);

        void setSourceUrl(const std::string &value);

        void print();

        void setEncryption(SegmentEncryption enc);

    public:
        std::string mUri = "";
        uint64_t startTime = 0;
        int64_t duration = -1;
        uint64_t sequence = 0;
        static const int SEQUENCE_INVALID;
        static const int SEQUENCE_FIRST;
        SegmentEncryption encryption;

        std::shared_ptr<segment> init_section{nullptr};

    };
}


#endif //FRAMEWORK_SEGMENT_H
