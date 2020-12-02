//
// Created by moqi on 2019-07-05.
//

#ifndef FRAMEWORK_AVPACKET_H
#define FRAMEWORK_AVPACKET_H

#include "base/media/IAFPacket.h"
#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/encryption_info.h>
}

#ifdef __APPLE__
class PBAFFrame;
#endif

class AVAFPacket : public IAFPacket {
public:
    attribute_deprecated explicit AVAFPacket(AVPacket &pkt, bool isProtected = false);

    explicit AVAFPacket(AVPacket *pkt, bool isProtected = false);

    explicit AVAFPacket(AVPacket **pkt, bool isProtected = false);

    ~AVAFPacket() override;

    void setDiscard(bool discard) override;

    uint8_t *getData() override;

    bool isProtected() override
    {
        return mIsProtected;
    }

    void setProtected() override
    {
        mIsProtected = true;
    }

    std::unique_ptr<IAFPacket> clone() const override;

    int64_t getSize() override;

    AVPacket *ToAVPacket();

    explicit operator AVPacket *();

    void setMagicKey(const std::string &key) override
    {
        if (mMagicKey.empty()) {
            mMagicKey = key;
        }
    }

    std::string getMagicKey() override
    {
        return mMagicKey;
    }

    bool getEncryptionInfo(EncryptionInfo* dst) override;

private:
    AVPacket *mpkt{nullptr};
    bool mIsProtected;
    std::string mMagicKey{};
    AVEncryptionInfo *mAVEncryptionInfo{nullptr};

    void copyInfo();
};


class CICADA_CPLUS_EXTERN AVAFFrame : public IAFFrame {
public:

    explicit AVAFFrame(const AFFrameInfo &info, const uint8_t **data, const int *lineSize, int lineNums,
                       IAFFrame::FrameType type = FrameTypeUnknown);

    explicit AVAFFrame(AVFrame *frame, FrameType type = FrameTypeUnknown);

    explicit AVAFFrame(AVFrame **frame, FrameType type = FrameTypeUnknown);

    ~AVAFFrame() override;

    std::unique_ptr<IAFFrame> clone() override;

    uint8_t **getData() override;

    int *getLineSize() override;

    FrameType getType() override;

    AVFrame *ToAVFrame();

    explicit operator AVFrame *() const;

#ifdef __APPLE__
    explicit operator PBAFFrame *();
#endif

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
    auto *avafPacket = dynamic_cast<AVAFPacket *>(packet);
    if (avafPacket) {
        return static_cast<AVPacket *>(*(avafPacket));
    }
    return nullptr;
}


#endif//FRAMEWORK_AVPACKET_H
