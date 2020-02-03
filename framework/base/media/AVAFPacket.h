//
// Created by moqi on 2019-07-05.
//

#ifndef FRAMEWORK_AVPACKET_H
#define FRAMEWORK_AVPACKET_H

#include "base/media/IAFPacket.h"

extern "C" {
#include <libavcodec/avcodec.h>
};


class AVAFPacket : public IAFPacket {
public:
    attribute_deprecated explicit AVAFPacket(AVPacket &pkt);

    explicit AVAFPacket(AVPacket *pkt);

    explicit AVAFPacket(AVPacket **pkt);

    ~AVAFPacket() override;

    void setDiscard(bool discard) override;

    uint8_t *getData() override;

    std::unique_ptr<IAFPacket> clone() override;

    int64_t getSize() override;

    AVPacket *ToAVPacket();

    explicit operator AVPacket *();

private:
    AVPacket *mpkt{nullptr};

    void copyInfo();
};


class AVAFFrame : public IAFFrame {
public:
    explicit AVAFFrame(AVFrame *frame, FrameType type = FrameTypeUnknown);

    explicit AVAFFrame(AVFrame **frame, FrameType type = FrameTypeUnknown);

    ~AVAFFrame() override;

    std::unique_ptr<IAFFrame> clone() override;

    uint8_t **getData() override;

    int *getLineSize() override;

    FrameType getType() override;

    AVFrame *ToAVFrame();

    explicit operator AVFrame *() const;

    void updateInfo();


private:
    AVFrame *mAvFrame;
    FrameType mType = FrameTypeUnknown;

    void copyInfo();
};

static inline AVFrame *getAVFrame(IAFFrame *frame)
{
    auto *avafFrame = dynamic_cast<AVAFFrame *>(frame);
    if (avafFrame) {
        return static_cast<AVFrame *>(*(avafFrame));
    }
    return nullptr;
}

static inline AVPacket *getAVPacket(IAFPacket *packet)
{
    auto * avafPacket = dynamic_cast<AVAFPacket *>(packet);
    if (avafPacket) {
        return static_cast<AVPacket *>(*(avafPacket));
    }
    return nullptr;

}


#endif //FRAMEWORK_AVPACKET_H
