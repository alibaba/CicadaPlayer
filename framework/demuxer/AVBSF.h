//
// Created by moqi on 2019-07-08.
//

#ifndef FRAMEWORK_AVBSF_H
#define FRAMEWORK_AVBSF_H

#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
}

struct AVBSFContext;
struct AVCodecParameters;
struct AVPacket;

namespace Cicada{
    class IAVBSF {
    public:
        virtual ~IAVBSF() = default;

        virtual int init(const std::string &name, AVCodecParameters *codecpar) = 0;

        /*
         * EAGAIN
         * EINVAL : send data after flush
         */
        virtual int push(AVPacket *pkt) = 0;

        virtual int pull(AVPacket *pkt) = 0;
    };

    class AVBSF : public IAVBSF {
    public:
        AVBSF();

        ~AVBSF() override;

        int init(const std::string &name, AVCodecParameters *codecpar) override;

        int push(AVPacket *pkt) override;

        int pull(AVPacket *pkt) override;


    private:
        AVBSFContext *bsfContext = nullptr;

    };

    class AFAVBSF : public IAVBSF {
    public:
        AFAVBSF();

        ~AFAVBSF() override;

        int init(const std::string &name, AVCodecParameters *codecpar) override;

        int push(AVPacket *pkt) override;

        int pull(AVPacket *pkt) override;

    private:
        int get_packet(AVPacket **pkt);

    private:
        bool mBNeedParser = false;
        AVPacket *mPkt{};
        bool bEof{};
        AVCodecID mCodecId{AV_CODEC_ID_NONE};
    };

    class IAVBSFFactory {
    public:
        static IAVBSF *create(const std::string &name);
    };
}


#endif //FRAMEWORK_AVBSF_H
