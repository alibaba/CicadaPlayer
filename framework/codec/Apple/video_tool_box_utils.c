//
// Created by moqi on 2019-08-28.
//

#include "video_tool_box_utils.h"
#include <VideoToolbox/VideoToolbox.h>
#include <config.h>
#include <libavcodec/bytestream.h>
#include <libavcodec/h264_parse.h>
#include <libavcodec/hevc_parse.h>
#include <utils/ffmpeg_utils.h>
#include <utils/frame_work_log.h>

#ifndef kVTVideoDecoderSpecification_RequireHardwareAcceleratedVideoDecoder
    #define kVTVideoDecoderSpecification_RequireHardwareAcceleratedVideoDecoder CFSTR("RequireHardwareAcceleratedVideoDecoder")
#endif
#define AV_W8(p, v) *(p) = (v)

#define VIDEOTOOLBOX_ESDS_EXTRADATA_PADDING  12

static void videotoolbox_write_mp4_descr_length(PutByteContext *pb, int length)
{
    int i;
    uint8_t b;

    for (i = 3; i >= 0; i--) {
        b = (length >> (i * 7)) & 0x7F;

        if (i != 0)
            b |= 0x80;

        bytestream2_put_byteu(pb, b);
    }
}

static CFDataRef videotoolbox_esds_extradata_create(const uint8_t *pData, int size)
{
    CFDataRef data;
    uint8_t *rw_extradata;
    PutByteContext pb;
    int full_size = 3 + 5 + 13 + 5 + size + 3;
    // ES_DescrTag data + DecoderConfigDescrTag + data + DecSpecificInfoTag + size + SLConfigDescriptor
    int config_size = 13 + 5 + size;
    int s;

    if (!(rw_extradata = av_mallocz(full_size + VIDEOTOOLBOX_ESDS_EXTRADATA_PADDING)))
        return NULL;

    bytestream2_init_writer(&pb, rw_extradata, full_size + VIDEOTOOLBOX_ESDS_EXTRADATA_PADDING);
    bytestream2_put_byteu(&pb, 0);        // version
    bytestream2_put_ne24(&pb, 0);         // flags
    // elementary stream descriptor
    bytestream2_put_byteu(&pb, 0x03);     // ES_DescrTag
    videotoolbox_write_mp4_descr_length(&pb, full_size);
    bytestream2_put_ne16(&pb, 0);         // esid
    bytestream2_put_byteu(&pb, 0);        // stream priority (0-32)
    // decoder configuration descriptor
    bytestream2_put_byteu(&pb, 0x04);     // DecoderConfigDescrTag
    videotoolbox_write_mp4_descr_length(&pb, config_size);
    bytestream2_put_byteu(&pb, 32);       // object type indication. 32 = AV_CODEC_ID_MPEG4
    bytestream2_put_byteu(&pb, 0x11);     // stream type
    bytestream2_put_ne24(&pb, 0);         // buffer size
    bytestream2_put_ne32(&pb, 0);         // max bitrate
    bytestream2_put_ne32(&pb, 0);         // avg bitrate
    // decoder specific descriptor
    bytestream2_put_byteu(&pb, 0x05);     ///< DecSpecificInfoTag
    videotoolbox_write_mp4_descr_length(&pb, size);
    bytestream2_put_buffer(&pb, pData, size);
    // SLConfigDescriptor
    bytestream2_put_byteu(&pb, 0x06);     // SLConfigDescrTag
    bytestream2_put_byteu(&pb, 0x01);     // length
    bytestream2_put_byteu(&pb, 0x02);     //
    s = bytestream2_size_p(&pb);
    data = CFDataCreate(kCFAllocatorDefault, rw_extradata, s);
    av_freep(&rw_extradata);
    return data;
}

int parser_extradata(const uint8_t *pData, int size, parserInfo *pInfo, enum AFCodecID codecId)
{
    int ret = -EINVAL, i;
    enum AVCodecID avCodecId = CodecID2AVCodecID(codecId);
    AVCodec *codec = avcodec_find_decoder(avCodecId);

    if (codec == NULL) {
        return -EINVAL;
    }

    AVCodecContext *avctx = avcodec_alloc_context3(codec);

    if (avCodecId == AV_CODEC_ID_H264) {
#if CONFIG_H264_PARSER
        H264ParamSets ps;
        const PPS *pps = NULL;
        const SPS *sps = NULL;
        int is_avc = 0;
        int nal_length_size = 0;
        memset(&ps, 0, sizeof(ps));
        ret = ff_h264_decode_extradata(pData, size, &ps, &is_avc, &nal_length_size, 0, avctx);
        avcodec_free_context(&avctx);

        if (ret < 0) {
            return -EINVAL;
        }

        for (i = 0; i < MAX_PPS_COUNT; i++) {
            if (ps.pps_list[i]) {
                pps = (const PPS *) ps.pps_list[i]->data;
                break;
            }
        }

        if (pps) {
            if (ps.sps_list[pps->sps_id]) {
                sps = (const SPS *) ps.sps_list[pps->sps_id]->data;
            }
        }

        if (pInfo && sps) {
            pInfo->width = (sps->mb_width) * 16 - (sps->crop_left + sps->crop_right);
            pInfo->height = (sps->mb_height) * 16 - (sps->crop_bottom + sps->crop_top);
            ret = 0;
        } else
            ret = -EINVAL;

        ff_h264_ps_uninit(&ps);
#else
        ret = -EINVAL;
#endif
    } else if (avCodecId == AV_CODEC_ID_HEVC) {
#if CONFIG_HEVC_PARSER
        HEVCParamSets ps;
#if (LIBAVCODEC_VERSION_MAJOR < 58)
        HEVCSEIContext sei;
#else
        HEVCSEI sei;
#endif
        const HEVCSPS *sps = NULL;
        int is_nalff = 0;
        int nal_length_size = 0;
        memset(&ps, 0, sizeof(ps));
        memset(&sei, 0, sizeof(sei));
        ret = ff_hevc_decode_extradata(pData, size,
                                       &ps, &sei, &is_nalff, &nal_length_size, 0, 1, avctx);
        avcodec_free_context(&avctx);

        if (ret < 0) {
            return -EINVAL;
        }

        sps = (const HEVCSPS *) ps.sps_list[0]->data;

        if (pInfo && sps) {
            pInfo->width = sps->width;
            pInfo->height = sps->height;
            ret = 0;
        } else
            ret = -EINVAL;

        ff_hevc_ps_uninit(&ps);
#else
        ret = -EINVAL;
#endif
    }

    return ret;
}
#if 0
static CFDataRef ff_videotoolbox_avcc_extradata_create(const uint8_t *pData, int size, parserInfo *pInfo)
{
    int ret;
    H264ParamSets ps;
    const PPS *pps = NULL;
    const SPS *sps = NULL;
    int is_avc = 0;
    int nal_length_size = 0;
    int i;
    memset(&ps, 0, sizeof(ps));
    AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    AVCodecContext *avctx = avcodec_alloc_context3(codec);
    ret = ff_h264_decode_extradata(pData, size, &ps, &is_avc, &nal_length_size, 0, avctx);
    avcodec_free_context(&avctx);

    if (ret < 0)
        return NULL;

    for (i = 0; i < MAX_PPS_COUNT; i++) {
        if (ps.pps_list[i]) {
            pps = (const PPS *) ps.pps_list[i]->data;
            break;
        }
    }

    if (pps) {
        if (ps.sps_list[pps->sps_id]) {
            sps = (const SPS *) ps.sps_list[pps->sps_id]->data;
        }
    }

    AF_LOGD("sps->num_reorder_frames is %d\n", sps->num_reorder_frames);
    CFDataRef data = NULL;

    if (sps && pps) {
        uint8_t *p;
        int vt_extradata_size = (int) (6 + 2 + sps->data_size + 3 + pps->data_size);
        uint8_t *vt_extradata = av_malloc(vt_extradata_size);

        if (!vt_extradata)
            return NULL;

        p = vt_extradata;
        AV_W8(p + 0, 1); /* version */
        AV_W8(p + 1, sps->data[1]); /* profile */
        AV_W8(p + 2, sps->data[2]); /* profile compat */
        AV_W8(p + 3, sps->data[3]); /* level */
        AV_W8(p + 4, 0xff); /* 6 bits reserved (111111) + 2 bits nal size length - 3 (11) */
        AV_W8(p + 5, 0xe1); /* 3 bits reserved (111) + 5 bits number of sps (00001) */
        AV_WB16(p + 6, sps->data_size);
        memcpy(p + 8, sps->data, sps->data_size);
        p += 8 + sps->data_size;
        AV_W8(p + 0, 1); /* number of pps */
        AV_WB16(p + 1, pps->data_size);
        memcpy(p + 3, pps->data, pps->data_size);
        p += 3 + pps->data_size;
        av_assert0(p - vt_extradata == vt_extradata_size);
        data = CFDataCreate(kCFAllocatorDefault, vt_extradata, vt_extradata_size);
        av_free(vt_extradata);
    }

    if (pInfo) {
        pInfo->width = (sps->mb_width) * 16 - (sps->crop_left + sps->crop_right);
        pInfo->height = (sps->mb_height) * 16 - (sps->crop_bottom + sps->crop_top);
    }

    return data;
}
static CFDataRef ff_videotoolbox_hvcc_extradata_create(const uint8_t *pData, int size, parserInfo *pInfo)
{
    int ret;
    HEVCParamSets ps;
#if (LIBAVCODEC_VERSION_MAJOR < 58)
    HEVCSEIContext sei;
#else
    HEVCSEI sei;
#endif
    const HEVCVPS *vps = NULL;
    const HEVCPPS *pps = NULL;
    const HEVCSPS *sps = NULL;
    int is_nalff = 0;
    int nal_length_size = 0;
    memset(&ps, 0, sizeof(ps));
    memset(&sei, 0, sizeof(sei));
    AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_HEVC);
    AVCodecContext *avctx = avcodec_alloc_context3(codec);
    ret = ff_hevc_decode_extradata(pData, size,
                                   &ps, &sei, &is_nalff, &nal_length_size, 0, 1, avctx);
    avcodec_free_context(&avctx);
    vps = (const HEVCVPS *) ps.vps_list[0]->data;
    sps = (const HEVCSPS *) ps.sps_list[0]->data;
    int i, num_pps = 0;
    pps = ps.pps;
    PTLCommon ptlc = vps->ptl.general_ptl;
    VUI vui = sps->vui;
    uint8_t parallelismType;
    CFDataRef data = NULL;
    uint8_t *p;
    int vt_extradata_size = 23 + 5 + vps->data_size + 5 + sps->data_size + 3;
    uint8_t *vt_extradata;

    for (i = 0; i < MAX_PPS_COUNT; i++) {
        if (ps.pps_list[i]) {
            const HEVCPPS *pps = (const HEVCPPS *) ps.pps_list[i]->data;
            vt_extradata_size += 2 + pps->data_size;
            num_pps++;
        }
    }

    vt_extradata = av_malloc(vt_extradata_size);

    if (!vt_extradata)
        return NULL;

    p = vt_extradata;
    /* unsigned int(8) configurationVersion = 1; */
    AV_W8(p + 0, 1);
    /*
     * unsigned int(2) general_profile_space;
     * unsigned int(1) general_tier_flag;
     * unsigned int(5) general_profile_idc;
     */
    AV_W8(p + 1, ptlc.profile_space << 6 |
          ptlc.tier_flag << 5 |
          ptlc.profile_idc);
    /* unsigned int(32) general_profile_compatibility_flags; */
    memcpy(p + 2, ptlc.profile_compatibility_flag, 4);
    /* unsigned int(48) general_constraint_indicator_flags; */
    AV_W8(p + 6, ptlc.progressive_source_flag << 7 |
          ptlc.interlaced_source_flag << 6 |
          ptlc.non_packed_constraint_flag << 5 |
          ptlc.frame_only_constraint_flag << 4);
    AV_W8(p + 7, 0);
    AV_WN32(p + 8, 0);
    /* unsigned int(8) general_level_idc; */
    AV_W8(p + 12, ptlc.level_idc);
    /*
     * bit(4) reserved = ‘1111’b;
     * unsigned int(12) min_spatial_segmentation_idc;
     */
    AV_W8(p + 13, 0xf0 | (vui.min_spatial_segmentation_idc >> 4));
    AV_W8(p + 14, vui.min_spatial_segmentation_idc & 0xff);

    /*
     * bit(6) reserved = ‘111111’b;
     * unsigned int(2) parallelismType;
     */
    if (!vui.min_spatial_segmentation_idc)
        parallelismType = 0;
    else if (pps->entropy_coding_sync_enabled_flag && pps->tiles_enabled_flag)
        parallelismType = 0;
    else if (pps->entropy_coding_sync_enabled_flag)
        parallelismType = 3;
    else if (pps->tiles_enabled_flag)
        parallelismType = 2;
    else
        parallelismType = 1;

    AV_W8(p + 15, 0xfc | parallelismType);
    /*
     * bit(6) reserved = ‘111111’b;
     * unsigned int(2) chromaFormat;
     */
    AV_W8(p + 16, sps->chroma_format_idc | 0xfc);
    /*
     * bit(5) reserved = ‘11111’b;
     * unsigned int(3) bitDepthLumaMinus8;
     */
    AV_W8(p + 17, (sps->bit_depth - 8) | 0xfc);
    /*
     * bit(5) reserved = ‘11111’b;
     * unsigned int(3) bitDepthChromaMinus8;
     */
    AV_W8(p + 18, (sps->bit_depth_chroma - 8) | 0xfc);
    /* bit(16) avgFrameRate; */
    AV_WB16(p + 19, 0);
    /*
     * bit(2) constantFrameRate;
     * bit(3) numTemporalLayers;
     * bit(1) temporalIdNested;
     * unsigned int(2) lengthSizeMinusOne;
     */
    AV_W8(p + 21, 0 << 6 |
          sps->max_sub_layers << 3 |
          sps->temporal_id_nesting_flag << 2 |
          3);
    /* unsigned int(8) numOfArrays; */
    AV_W8(p + 22, 3);
    p += 23;
    /* vps */
    /*
     * bit(1) array_completeness;
     * unsigned int(1) reserved = 0;
     * unsigned int(6) NAL_unit_type;
     */
    AV_W8(p, 1 << 7 |
          HEVC_NAL_VPS & 0x3f);
    /* unsigned int(16) numNalus; */
    AV_WB16(p + 1, 1);
    /* unsigned int(16) nalUnitLength; */
    AV_WB16(p + 3, vps->data_size);
    /* bit(8*nalUnitLength) nalUnit; */
    memcpy(p + 5, vps->data, vps->data_size);
    p += 5 + vps->data_size;
    /* sps */
    AV_W8(p, 1 << 7 |
          HEVC_NAL_SPS & 0x3f);
    AV_WB16(p + 1, 1);
    AV_WB16(p + 3, sps->data_size);
    memcpy(p + 5, sps->data, sps->data_size);
    p += 5 + sps->data_size;
    /* pps */
    AV_W8(p, 1 << 7 |
          HEVC_NAL_PPS & 0x3f);
    AV_WB16(p + 1, num_pps);
    p += 3;

    for (i = 0; i < MAX_PPS_COUNT; i++) {
        if (ps.pps_list[i]) {
            const HEVCPPS *pps = (const HEVCPPS *) ps.pps_list[i]->data;
            AV_WB16(p, pps->data_size);
            memcpy(p + 2, pps->data, pps->data_size);
            p += 2 + pps->data_size;
        }
    }

    av_assert0(p - vt_extradata == vt_extradata_size);
    data = CFDataCreate(kCFAllocatorDefault, vt_extradata, vt_extradata_size);
    av_free(vt_extradata);

    if (pInfo) {
        pInfo->width = sps->width;
        pInfo->height = sps->height;
    }

    return data;
}


CFDictionaryRef videotoolbox_decoder_config_create(CMVideoCodecType codec_type, const uint8_t *pData, int size, parserInfo *pInfo)
{
    CFMutableDictionaryRef config_info = CFDictionaryCreateMutable(kCFAllocatorDefault,
                                         0,
                                         &kCFTypeDictionaryKeyCallBacks,
                                         &kCFTypeDictionaryValueCallBacks);
//    CFDictionarySetValue(config_info,
//                         kVTVideoDecoderSpecification_RequireHardwareAcceleratedVideoDecoder,
//                         kCFBooleanTrue);

    if (size == 0) {
        return config_info;
    }

    CFMutableDictionaryRef avc_info;
    CFDataRef data = NULL;
    avc_info = CFDictionaryCreateMutable(kCFAllocatorDefault,
                                         1,
                                         &kCFTypeDictionaryKeyCallBacks,
                                         &kCFTypeDictionaryValueCallBacks);

    switch (codec_type) {
        case kCMVideoCodecType_MPEG4Video :
            data = videotoolbox_esds_extradata_create(pData, size);

            if (data)
                CFDictionarySetValue(avc_info, CFSTR("esds"), data);

            break;

        case kCMVideoCodecType_H264 :
            data = ff_videotoolbox_avcc_extradata_create(pData, size, pInfo);

            if (data)
                CFDictionarySetValue(avc_info, CFSTR("avcC"), data);

            break;

        case kCMVideoCodecType_HEVC :
            data = ff_videotoolbox_hvcc_extradata_create(pData, size, pInfo);

            if (data)
                CFDictionarySetValue(avc_info, CFSTR("hvcC"), data);

            break;

        default:
            break;
    }

    CFDictionarySetValue(config_info,
                         kCMFormatDescriptionExtension_SampleDescriptionExtensionAtoms,
                         avc_info);

    if (data)
        CFRelease(data);

    CFRelease(avc_info);
    return config_info;
}
#endif
CFDictionaryRef videotoolbox_buffer_attributes_create(int width, int height, OSType pix_fmt)
{
    CFMutableDictionaryRef buffer_attributes;
    CFMutableDictionaryRef io_surface_properties;
    CFNumberRef cv_pix_fmt;
    CFNumberRef w;
    CFNumberRef h;
    w = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &width);
    h = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &height);
    cv_pix_fmt = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &pix_fmt);
    buffer_attributes = CFDictionaryCreateMutable(kCFAllocatorDefault,
                        4,
                        &kCFTypeDictionaryKeyCallBacks,
                        &kCFTypeDictionaryValueCallBacks);
    io_surface_properties = CFDictionaryCreateMutable(kCFAllocatorDefault,
                            0,
                            &kCFTypeDictionaryKeyCallBacks,
                            &kCFTypeDictionaryValueCallBacks);

    if (pix_fmt)
        CFDictionarySetValue(buffer_attributes, kCVPixelBufferPixelFormatTypeKey, cv_pix_fmt);

    CFDictionarySetValue(buffer_attributes, kCVPixelBufferIOSurfacePropertiesKey, io_surface_properties);
    CFDictionarySetValue(buffer_attributes, kCVPixelBufferWidthKey, w);
    CFDictionarySetValue(buffer_attributes, kCVPixelBufferHeightKey, h);
#if TARGET_OS_IPHONE
    CFDictionarySetValue(buffer_attributes, kCVPixelBufferOpenGLESCompatibilityKey, kCFBooleanTrue);
#else
    CFDictionarySetValue(buffer_attributes, kCVPixelBufferIOSurfaceOpenGLTextureCompatibilityKey, kCFBooleanTrue);
#endif
    CFRelease(io_surface_properties);
    CFRelease(cv_pix_fmt);
    CFRelease(w);
    CFRelease(h);
    return buffer_attributes;
}
