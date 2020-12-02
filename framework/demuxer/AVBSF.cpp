//
// Created by moqi on 2019-07-08.
//
#define LOG_TAG "AVBSF"
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avc.h>
#include <libavformat/hevc.h>
}

#include <utils/frame_work_log.h>
#include "AVBSF.h"
#include "utils/ffmpeg_utils.h"
#include "AdtsBSF.h"

namespace Cicada {
    AVBSF::AVBSF()
    {
    }

    AVBSF::~AVBSF()
    {
        av_bsf_free(&bsfContext);
    }

    int AVBSF::init(const std::string &name, AVCodecParameters *codecpar)
    {
        const AVBitStreamFilter *bsf = av_bsf_get_by_name(name.c_str());

        if (!bsf) {
            AF_LOGE("%s bsf not found\n", name.c_str());
            return -EPERM;
        }

        int ret = av_bsf_alloc(bsf, &bsfContext);

        if (ret < 0) {
            AF_LOGE("Cannot alloc BSF!\n");
            return -ENOMEM;
        }

        avcodec_parameters_copy(bsfContext->par_in, codecpar);
        ret = av_bsf_init(bsfContext);

        if (ret < 0) {
            AF_LOGE("Error initializing bitstream filter: %s\n", bsf->name);
            return ret;
        }

        ret = avcodec_parameters_copy(codecpar, bsfContext->par_out);

        if (ret < 0) {
            return ret;
        }

        return 0;
    }

    int AVBSF::push(AVPacket *pkt)
    {
        int ret = av_bsf_send_packet(bsfContext, pkt);

        if (ret < 0) {
            AF_LOGE("av_bsf_send_packet error %d (%s)\n", ret,  getErrorString(ret));
        }

        return ret;
    }

    int AVBSF::pull(AVPacket *pkt)
    {
        int ret = av_bsf_receive_packet(bsfContext, pkt);

        if (ret == AVERROR_EOF) {
            return 0;
        } else if (ret < 0) {
            AF_LOGE("av_bsf_receive_packet error %d (%s)\n", ret,  getErrorString(ret));
            return ret;
        } else {
            return pkt->size;
        }
    }

    AFAVBSF::AFAVBSF()
    {
        mPkt = av_packet_alloc();
        av_init_packet(mPkt);
    }

    AFAVBSF::~AFAVBSF()
    {
        av_packet_free(&mPkt);
    }

    int AFAVBSF::init(const std::string &name, AVCodecParameters *codecpar)
    {
        if (name != "h26xAnnexb2xVcc") {
            return -EINVAL;
        }

        if (codecpar->codec_id != AV_CODEC_ID_H264 && codecpar->codec_id != AV_CODEC_ID_HEVC) {
            return -EINVAL;
        }

        mBNeedParser = updateH26xHeader2xxc(codecpar);
        mCodecId = codecpar->codec_id;
        return 0;
    }

    int AFAVBSF::push(AVPacket *pkt)
    {
        if (pkt == nullptr) {
            bEof = true;
            return 0;
        }

        if (bEof) {
            return -EINVAL;
        }

        if (mPkt->data || mPkt->side_data_elems) {
            return -EAGAIN;
        }

        av_packet_move_ref(mPkt, pkt);
        return 0;
    }

    int AFAVBSF::pull(AVPacket *pkt)
    {
        AVPacket *in = nullptr;
        int ret = get_packet(&in);

        if (ret < 0) {
            if (ret == AVERROR_EOF) {
                return 0;
            }

            return ret;
        }

        if (!mBNeedParser) {
            av_packet_move_ref(pkt, in);
            return pkt->size;
        }

        uint8_t *reformatted_data = nullptr;
        int size = in->size;

        if (mCodecId == AV_CODEC_ID_H264) {
            ff_avc_parse_nal_units_buf(in->data, &reformatted_data, &size);
        } else if (mCodecId == AV_CODEC_ID_HEVC) {
            ff_hevc_annexb2mp4_buf(in->data, &reformatted_data, &size, 0, nullptr);
        } else {
            AF_LOGE("error codec id\n");
            av_packet_free(&in);
            return -EINVAL;
        }

        if (reformatted_data) {
            av_packet_from_data(pkt, reformatted_data, size);
            av_packet_copy_props(pkt, in);
            av_packet_free(&in);
            return pkt->size;
        }

        av_packet_free(&in);
        return -EINVAL;
    }

    int AFAVBSF::get_packet(AVPacket **pkt)
    {
        if (bEof) {
            return 0;
        }

        if (!mPkt->data && !mPkt->side_data_elems) {
            return AVERROR(EAGAIN);
        }

        AVPacket *tmp_pkt;
        tmp_pkt = av_packet_alloc();

        if (!tmp_pkt) {
            return AVERROR(ENOMEM);
        }

        *pkt = mPkt;
        mPkt = tmp_pkt;
        return 0;
    }

    IAVBSF *IAVBSFFactory::create(const std::string &name)
    {
        if (name == "h26xAnnexb2xVcc") {
            return new AFAVBSF();
        }else if(name == "latm2Adts") {
            return new AdtsBSF();
        }

        return new AVBSF();
    }
}
