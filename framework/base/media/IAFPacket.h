//
// Created by moqi on 2019-07-05.
//

#ifndef FRAMEWORK_IAFPACKET_H
#define FRAMEWORK_IAFPACKET_H


#include <cstdint>
//#include <functional>
#include <vector>
#include <memory>
#include <utils/frame_work_log.h>
#include <cstring>

extern "C" {
//#include <libavutil/rational.h>
};


struct AVRational;

struct AFRational {
    int num; ///< Numerator
    int den; ///< Denominator

    AFRational &operator=(AVRational rational);
};
#define AF_PKT_FLAG_KEY     0x0001 ///< The packet contains a keyframe
#define AF_PKT_FLAG_CORRUPT 0x0002 ///< The packet content is corrupted
class CICADA_CPLUS_EXTERN IAFPacket {
public:
    struct packetInfo {
        int streamIndex;
        int64_t pts;
        int64_t dts;
        int flags;
        int duration;
        int64_t pos;
        int64_t timePosition;
        bool seamlessPoint;

        uint8_t *extra_data;
        int extra_data_size;

        void dump();

        ~packetInfo()
        {
            delete[](extra_data);

        }
    };

public:
    // TODO: std::move constructor
    IAFPacket() = default;

    virtual ~IAFPacket() = default;

    virtual std::unique_ptr<IAFPacket> clone() = 0;

    //TODO renturn const uint8_t, now for framework use
    virtual uint8_t *getData() = 0;

    virtual int64_t getSize() = 0;

    virtual void setDiscard(bool discard)
    {
        mbDiscard = discard;
    };

    virtual bool getDiscard()
    {
        return mbDiscard;
    }

    packetInfo &getInfo();

    void setExtraData(const uint8_t *extra_data, int extra_data_size)
    {
        if (extra_data) {
            delete[] mInfo.extra_data;
            mInfo.extra_data = new uint8_t[extra_data_size];
            mInfo.extra_data_size = extra_data_size;
            memcpy(mInfo.extra_data, extra_data, mInfo.extra_data_size);
        }
    }


protected:
    packetInfo mInfo{};
    bool mbDiscard{};

};

class CICADA_CPLUS_EXTERN IAFFrame {
public:
    enum FrameType {
        FrameTypeUnknown,
        FrameTypeVideo,
        FrameTypeAudio
    };
    struct videoInfo {
        int width;
        int height;
        int rotate;
        double dar;

        AFRational sample_aspect_ratio;
        size_t crop_top;
        size_t crop_bottom;
        size_t crop_left;
        size_t crop_right;

        int colorRange;
        int colorSpace;
    };
    struct audioInfo {
        int nb_samples;
        int channels;
        int sample_rate;
        uint64_t channel_layout;
        int format;

        bool operator==(const audioInfo &info) const
        {
            return this->sample_rate == info.sample_rate
                   && this->channels == info.channels
                   && this->format == info.format
                   && this->channel_layout == info.channel_layout
         //          && this->nb_samples == info.nb_samples
                   ;
        }

        bool operator!=(const audioInfo &info) const
        {
            return !operator==(info);
        }
    };
    struct AFFrameInfo {
        int64_t pts;
        int64_t pkt_dts;
        int64_t duration;
        bool key;
        int format;
        union {
            videoInfo video;
            audioInfo audio;
        };
    };
public:
    IAFFrame() = default;

    virtual ~IAFFrame() = default;

    virtual std::unique_ptr<IAFFrame> clone() = 0;

    virtual uint8_t **getData() = 0;

    virtual int *getLineSize() = 0;

    virtual FrameType getType() = 0;

    virtual void setDiscard(bool discard)
    {

    }

    AFFrameInfo &getInfo();

    void dump();

protected:
    AFFrameInfo mInfo{};

};


#endif //FRAMEWORK_IAFPACKET_H
