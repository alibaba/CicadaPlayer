//
// Created by moqi on 2019-08-20.
//

#include <drm/DrmInfo.h>
#include "decoderFactory.h"

#ifdef ANDROID

    #include "Android/mediaCodecDecoder.h"

#endif

#ifdef __APPLE__

    #include "Apple/AppleVideoToolBox.h"

#endif

#include "avcodecDecoder.h"

using namespace Cicada;
using namespace std;

unique_ptr<Cicada::IDecoder> decoderFactory::create(const Stream_meta& meta, uint64_t flags, int maxSize,
                                                    const DrmInfo *drmInfo)
{
    IDecoder *decoder = codecPrototype::create(meta, flags, maxSize, drmInfo);

    if (decoder != nullptr) {
        return unique_ptr<IDecoder>(decoder);
    }

    return createBuildIn(meta.codec, flags, drmInfo);
}

unique_ptr<IDecoder> decoderFactory::createBuildIn(const AFCodecID &codec, uint64_t flags,
                                                   const DrmInfo *drmInfo)
{
    if (flags & DECFLAG_HW) {
#ifdef ANDROID
#ifdef ENABLE_MEDIA_CODEC_DECODER
        return std::unique_ptr<IDecoder>(new mediaCodecDecoder());
#endif
#endif
#ifdef __APPLE__
#ifdef ENABLE_VTB_DECODER

        if (AFVTBDecoder::is_supported(codec)) {
            return unique_ptr<IDecoder>(new AFVTBDecoder());
        }

#endif
#endif
    }

    if (flags & DECFLAG_SW) {
#ifdef ENABLE_AVCODEC_DECODER
        return unique_ptr<IDecoder>(new avcodecDecoder());
#endif
    }

    return nullptr;
}

