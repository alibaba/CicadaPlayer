//
// Created by SuperMan on 12/4/20.
//

#ifndef SOURCE_SMPRECORDERSET_H
#define SOURCE_SMPRECORDERSET_H


#include <cstdint>
#include <atomic>

class DecodeFirstFrameInfo {
public:
    DecodeFirstFrameInfo() = default;

    ~DecodeFirstFrameInfo() = default;

    int64_t sendFirstPacketTimeMs{INT64_MIN};
    int64_t getFirstFrameTimeMs{INT64_MIN};
    bool isFirstPacketSendToDecoder{false};
    bool waitFirstFrame{false};
    int64_t firstPacketSize{0};
    int64_t firstPacketPts{INT64_MIN};

public:
    int64_t getDecodeFirstFrameCost() {
        if (getFirstFrameTimeMs != INT64_MIN && sendFirstPacketTimeMs != INT64_MIN) {
            return getFirstFrameTimeMs - sendFirstPacketTimeMs;
        } else {
            return INT64_MIN;
        }
    }

    void reset() {
        sendFirstPacketTimeMs = INT64_MIN;
        getFirstFrameTimeMs = INT64_MIN;
        isFirstPacketSendToDecoder = false;
        waitFirstFrame = false;
        firstPacketSize = 0;
        firstPacketPts = INT64_MIN;
    }
};

class SMPRecorderSet {

public:
    SMPRecorderSet() = default;

    ~SMPRecorderSet() = default;

    void reset() {
        createAudioDecoderCostMs = INT64_MIN;
        createVideoDecoderCostMs = INT64_MIN;

        decodeFirstAudioFrameInfo.reset();
        decodeFirstVideoFrameInfo.reset();
    }

public:
    std::atomic<int64_t> createAudioDecoderCostMs{INT64_MIN};
    std::atomic<int64_t> createVideoDecoderCostMs{INT64_MIN};

    DecodeFirstFrameInfo decodeFirstAudioFrameInfo{};
    DecodeFirstFrameInfo decodeFirstVideoFrameInfo{};

};


#endif //SOURCE_SMPRECORDERSET_H
