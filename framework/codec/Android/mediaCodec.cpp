#include "mediaCodec.h"

namespace Cicada {

    static const struct {
        OMX_VIDEO_AVCPROFILETYPE omx_profile;
        size_t profile_idc;
    } omx_to_profile_idc[] = {
        {OMX_VIDEO_AVCProfileBaseline, PROFILE_H264_BASELINE},
        {OMX_VIDEO_AVCProfileMain,     PROFILE_H264_MAIN},
        {OMX_VIDEO_AVCProfileExtended, PROFILE_H264_EXTENDED},
        {OMX_VIDEO_AVCProfileHigh,     PROFILE_H264_HIGH},
        {OMX_VIDEO_AVCProfileHigh10,   PROFILE_H264_HIGH_10},
        {OMX_VIDEO_AVCProfileHigh422,  PROFILE_H264_HIGH_422},
        {OMX_VIDEO_AVCProfileHigh444,  PROFILE_H264_HIGH_444},
    };

    size_t convert_omx_to_profile_idc(OMX_VIDEO_AVCPROFILETYPE profile_type)
    {
        size_t array_length = sizeof(omx_to_profile_idc) / sizeof(omx_to_profile_idc[0]);

        for (size_t i = 0; i < array_length; ++i) {
            if (omx_to_profile_idc[i].omx_profile == profile_type) {
                return omx_to_profile_idc[i].profile_idc;
            }
        }

        return 0;
    }

    bool OMXCodec_IsBlacklisted(const char *p_name, unsigned int i_name_len)
    {
        static const char *blacklisted_prefix[] = {
            /* ignore OpenCore software codecs */
            "OMX.PV.",
            /* The same sw codecs, renamed in ICS (perhaps also in honeycomb) */
            "OMX.google.",
            /* This one has been seen on HTC One V - it behaves like it works,
             * but FillBufferDone returns buffers filled with 0 bytes. The One V
             * has got a working OMX.qcom.video.decoder.avc instead though. */
            "OMX.ARICENT.",
            /* Use VC1 decoder for WMV3 for now */
            "OMX.SEC.WMV.Decoder",
            /* This decoder does work, but has an insane latency (leading to errors
             * about "main audio output playback way too late" and dropped frames).
             * At least Samsung Galaxy S III (where this decoder is present) has
             * got another one, OMX.SEC.mp3.dec, that works well and has a
             * sensible latency. (Also, even if that one isn't found, in general,
             * using SW codecs is usually more than fast enough for MP3.) */
            "OMX.SEC.MP3.Decoder",
            /* black screen */
            "OMX.MTK.VIDEO.DECODER.VC1",
            /* Not working or crashing (Samsung) */
            "OMX.SEC.vp8.dec",
            nullptr
        };
        static const char *blacklisted_suffix[] = {
            /* Codecs with DRM, that don't output plain YUV data but only
             * support direct rendering where the output can't be intercepted. */
            ".secure",
            /* Samsung sw decoders */
            ".sw.dec",
            /* Vivo sw decoders */
            ".hevcswvdec",

            nullptr
        };

        /* p_name is not '\0' terminated */

        for (const char **pp_bl_prefix = blacklisted_prefix; *pp_bl_prefix != nullptr;
                pp_bl_prefix++) {
            if (!strncmp(p_name, *pp_bl_prefix,
                         __MIN(strlen(*pp_bl_prefix), i_name_len))) {
                return true;
            }
        }

        for (const char **pp_bl_suffix = blacklisted_suffix; *pp_bl_suffix != nullptr;
                pp_bl_suffix++) {
            size_t i_suffix_len = strlen(*pp_bl_suffix);

            if (i_name_len > i_suffix_len
                    && !strncmp(p_name + i_name_len - i_suffix_len, *pp_bl_suffix,
                                i_suffix_len)) {
                return true;
            }
        }

        return false;
    }

    OMX_VIDEO_AVCPROFILETYPE selectprofiletype(int profile)
    {
        if (profile == OMX_VIDEO_AVCProfileBaseline) {
            return OMX_VIDEO_AVCProfileBaseline;
        }

        if (profile == OMX_VIDEO_AVCProfileMain) {
            return OMX_VIDEO_AVCProfileMain;
        }

        if (profile == OMX_VIDEO_AVCProfileExtended) {
            return OMX_VIDEO_AVCProfileExtended;
        }

        if (profile == OMX_VIDEO_AVCProfileHigh) {
            return OMX_VIDEO_AVCProfileHigh;
        }

        if (profile == OMX_VIDEO_AVCProfileHigh10) {
            return OMX_VIDEO_AVCProfileHigh10;
        }

        if (profile == OMX_VIDEO_AVCProfileHigh422) {
            return OMX_VIDEO_AVCProfileHigh422;
        }

        if (profile == OMX_VIDEO_AVCProfileHigh444) {
            return OMX_VIDEO_AVCProfileHigh444;
        }

        if (profile == OMX_VIDEO_AVCProfileKhronosExtensions) {
            return OMX_VIDEO_AVCProfileKhronosExtensions;
        }

        if (profile == OMX_VIDEO_AVCProfileVendorStartUnused) {
            return OMX_VIDEO_AVCProfileVendorStartUnused;
        }

        if (profile == OMX_VIDEO_AVCProfileMax) {
            return OMX_VIDEO_AVCProfileMax;
        }

        return OMX_VIDEO_AVCProfileMax;
    }
}