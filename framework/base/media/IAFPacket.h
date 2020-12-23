//
// Created by moqi on 2019-07-05.
//

#ifndef FRAMEWORK_IAFPACKET_H
#define FRAMEWORK_IAFPACKET_H


#include <cstdint>
//#include <functional>
#include <vector>
#include <list>
#include <memory>
#include <cstring>
#include <string>
extern "C" {
//#include <libavutil/rational.h>
};
#include <utils/CicadaType.h>

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

    struct SubsampleEncryptionInfo {
        unsigned int bytes_of_clear_data{0};
        unsigned int bytes_of_protected_data{0};
    };

    struct EncryptionInfo {
        std::string scheme{};

        uint32_t crypt_byte_block{0};
        uint32_t skip_byte_block{0};

        uint8_t *key_id{nullptr};
        uint32_t key_id_size{0};

        uint8_t *iv{nullptr};
        uint32_t iv_size{0};

        std::list<SubsampleEncryptionInfo> subsamples{};
        uint32_t subsample_count{0};

        ~EncryptionInfo()
        {
            if (!subsamples.empty()) {
                subsamples.clear();
            }
        }
    };

public:
    // TODO: std::move constructor
    IAFPacket() = default;

    virtual ~IAFPacket() = default;

    virtual std::unique_ptr<IAFPacket> clone() const = 0;

    //TODO renturn const uint8_t, now for framework use
    virtual uint8_t *getData() = 0;

    virtual int64_t getSize() = 0;

    virtual void setDiscard(bool discard)
    {
        mbDiscard = discard;
    }

    virtual bool getDiscard()
    {
        return mbDiscard;
    }

    virtual bool isProtected()
    {
        return false;
    }
    virtual void setProtected() = 0;

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

    virtual std::string getMagicKey()
    {
        return "";
    }

    virtual void setMagicKey(const std::string & key)
    {

    }

    virtual bool getEncryptionInfo(EncryptionInfo *dst)
    {
        return false;
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
        int format;

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
        int64_t timePosition;
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
        mbDiscard = discard;
    }

    virtual bool getDiscard()
    {
        return mbDiscard;
    }

    AFFrameInfo &getInfo();

    void setProtect(bool protect)
    {
        mbProtected = protect;
    }

    bool isProtected() const
    {
        return mbProtected;
    }

    void dump();

protected:
    AFFrameInfo mInfo{};
    bool mbDiscard{false};
    bool mbProtected{false};
};


#endif //FRAMEWORK_IAFPACKET_H
