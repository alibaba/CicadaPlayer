
#define LOG_TAG "AFVTBDecoder"

#include "AppleVideoToolBox.h"
#include "codec/utils_ios.h"
#include "utils/errors/framework_error.h"
#include "utils/timer.h"
#include <cinttypes>

#include "video_tool_box_utils.h"

using namespace std;
namespace Cicada {
    static const int IOS_OUTPUT_CACHE_FOR_B_FRAMES = 8;
    AFVTBDecoder AFVTBDecoder::se(0);

    AFVTBDecoder::AFVTBDecoder()
    {
#if TARGET_OS_IPHONE
        RegisterIOSNotificationObserver(this, (int) (IOSResignActive | IOSBecomeActive));
        mActive = IOSNotificationManager::Instance()->GetActiveStatus() != 0;
#if TARGET_OS_SIMULATOR
        mVTOutFmt = AF_PIX_FMT_YUV420P;
#endif
#elif TARGET_OS_OSX
        mVTOutFmt = AF_PIX_FMT_YUV420P;
#endif
    }

    AFVTBDecoder::~AFVTBDecoder()
    {
        if (mIsDummy) {
            return;
        }

        close();
#if TARGET_OS_IPHONE
        RemoveIOSNotificationObserver(this);
#endif
    }

    bool AFVTBDecoder::is_supported(enum AFCodecID codec)
    {
        if (codec != AF_CODEC_ID_H264 && codec != AF_CODEC_ID_HEVC) {
            return false;
        }

#ifdef NDEBUG

        if (codec == AF_CODEC_ID_HEVC) {
#if TARGET_OS_IPHONE

            if (__builtin_available(iOS 11.0, *))
#else
            if (__builtin_available(macOS 10.13, *))
#endif
            {
                return VTIsHardwareDecodeSupported(kCMVideoCodecType_HEVC);
            } else {
                return false;
            }
        }

#endif
        return true;
    }

    int AFVTBDecoder::init_decoder_internal()
    {
        mInputCount = 0;
        Stream_meta *meta = ((Stream_meta *) (*(mPInMeta)));
        int ret = gen_framework_errno(error_class_codec, codec_error_video_not_support);

        if (meta->codec == AF_CODEC_ID_H264) {
            mVideoCodecType = kCMVideoCodecType_H264;
        } else if (meta->codec == AF_CODEC_ID_MPEG4) {
            mVideoCodecType = kCMVideoCodecType_MPEG4Video;
        } else if (meta->codec == AF_CODEC_ID_HEVC) {
            mVideoCodecType = kCMVideoCodecType_HEVC;
        } else {
            return ret;
        }

        if (mVideoCodecType == kCMVideoCodecType_HEVC) {
#if TARGET_OS_IPHONE

            if (Cicada::GetIosVersion() < 11.0) {
                AF_LOGE("version below 11.0, not support hevc ios harddecoder.");
                return ret;
            }

#endif
        }

//        if (!VTIsHardwareDecodeSupported(mVideoCodecType))
//            return ret;
//        mGotFirstFrame = false;

        if (meta->extradata && meta->extradata_size > 0  && mActive) {
            ret = createDecompressionSession(meta->extradata, meta->extradata_size, meta->width, meta->height);

            if (ret < 0) {
                AF_LOGE("createDecompressionSession error\n");
                return ret;
            }
        }

        return 0;
    }

    int AFVTBDecoder::init_decoder(const Stream_meta *meta, void *voutObsr, uint64_t flags)
    {
        if (meta->pixel_fmt == AF_PIX_FMT_YUV422P || meta->pixel_fmt == AF_PIX_FMT_YUVJ422P) {
            return -ENOSPC;
        }

        mPInMeta = unique_ptr<streamMeta>(new streamMeta(meta));
        ((Stream_meta *) (*(mPInMeta)))->extradata = new uint8_t[meta->extradata_size];
        memcpy(((Stream_meta *) (*(mPInMeta)))->extradata, meta->extradata, ((Stream_meta *) (*(mPInMeta)))->extradata_size);
        ((Stream_meta *) (*(mPInMeta)))->lang = nullptr;
        ((Stream_meta *) (*(mPInMeta)))->description = nullptr;
        ((Stream_meta *) (*(mPInMeta)))->meta = nullptr;
        mInputCount = 0;

        if (meta->codec == AF_CODEC_ID_H264 /*|| meta->codec == AF_CODEC_ID_HEVC*/) {
            mParser = unique_ptr<bitStreamParser>(new bitStreamParser());
            int ret = mParser->init((Stream_meta *) (*(mPInMeta)));

            if (ret < 0) {
                mParser = nullptr;
            } else {
                mBUsePoc = true;
            }
        }

        int ret = init_decoder_internal();

        if (ret < 0) {
            return ret;
        }

        mFlags |= DECFLAG_HW;
        return 0;
    }

    void AFVTBDecoder::close_decoder()
    {
        if (!mVTDecompressSessionRef) {
            return;
        }

        VTDecompressionSessionInvalidate(mVTDecompressSessionRef);
        CFRelease(mVTDecompressSessionRef);
        mVTDecompressSessionRef = nullptr;
        CFRelease(mVideoFormatDesRef);
        mVideoFormatDesRef = nullptr;

        if (mDecoder_spec) {
            CFRelease(mDecoder_spec);
            mDecoder_spec = nullptr;
        }
        mPocErrorCount = 0;
    };

    int AFVTBDecoder::createDecompressionSession(uint8_t *pData, int size, int width, int height)
    {
        int rv = 0;

        if (!mVideoFormatDesRef) {
            rv = createVideoFormatDesc(pData, size, width, height, mDecoder_spec, mVideoFormatDesRef);
        }

        if (rv != 0) {
            AF_LOGE("createVideoFormatDescFromPacket failed rv %d", rv);
            return -EINVAL;
        }

        OSType pix_fmt = 0;

        if (mVTOutFmt == AF_PIX_FMT_YUV420P) {
            pix_fmt = kCVPixelFormatType_420YpCbCr8Planar;
        }

        CFDictionaryRef buf_attr = videotoolbox_buffer_attributes_create(width, height, pix_fmt);
        VTDecompressionOutputCallbackRecord outputCallbackRecord;
        outputCallbackRecord.decompressionOutputCallback = Cicada::AFVTBDecoder::decompressionOutputCallback;
        outputCallbackRecord.decompressionOutputRefCon = this;
        rv = VTDecompressionSessionCreate(kCFAllocatorDefault,
                                          mVideoFormatDesRef,
                                          mDecoder_spec,
                                          buf_attr,
                                          &outputCallbackRecord,
                                          &mVTDecompressSessionRef);

        if (rv != 0) {
            AF_LOGE("create seesion failed rv %d", rv);
            CFRelease(buf_attr);

            if (mVideoFormatDesRef) {
                CFRelease(mVideoFormatDesRef);
                mVideoFormatDesRef = nullptr;
            }

            if (mDecoder_spec) {
                CFRelease(mDecoder_spec);
                mDecoder_spec = nullptr;
            }

            return -EINVAL;
        }

        CFRelease(buf_attr);
        return 0;
    }

    static void dict_set_string(CFMutableDictionaryRef dict, CFStringRef key, const char *value)
    {
        CFStringRef string;
        string = CFStringCreateWithCString(NULL, value, kCFStringEncodingASCII);
        CFDictionarySetValue(dict, key, string);
        CFRelease(string);
    }

    static void dict_set_boolean(CFMutableDictionaryRef dict, CFStringRef key, bool value)
    {
        CFDictionarySetValue(dict, key, value ? kCFBooleanTrue : kCFBooleanFalse);
    }

    static void dict_set_data(CFMutableDictionaryRef dict, CFStringRef key, uint8_t *value, uint64_t length)
    {
        CFDataRef data;
        data = CFDataCreate(NULL, value, (CFIndex) length);
        CFDictionarySetValue(dict, key, data);
        CFRelease(data);
    }

    static void dict_set_object(CFMutableDictionaryRef dict, CFStringRef key, CFTypeRef *value)
    {
        CFDictionarySetValue(dict, key, value);
    }

    static void dict_set_i32(CFMutableDictionaryRef dict, CFStringRef key,
                             int32_t value)
    {
        CFNumberRef number;
        number = CFNumberCreate(nullptr, kCFNumberSInt32Type, &value);
        CFDictionarySetValue(dict, key, number);
        CFRelease(number);
    }

    int AFVTBDecoder::createVideoFormatDesc(const uint8_t *pData, int size, int width, int height, CFDictionaryRef &decoder_spec,
                                            CMFormatDescriptionRef &cm_fmt_desc)
    {
        OSStatus status;
        int ret;

        if (pData == nullptr || size == 0) {
            return -EINVAL;
        }

        CFMutableDictionaryRef par = CFDictionaryCreateMutable(nullptr, 0, &kCFTypeDictionaryKeyCallBacks,
                                     &kCFTypeDictionaryValueCallBacks);
        CFMutableDictionaryRef atoms = CFDictionaryCreateMutable(nullptr, 0, &kCFTypeDictionaryKeyCallBacks,
                                       &kCFTypeDictionaryValueCallBacks);
        CFMutableDictionaryRef extensions = CFDictionaryCreateMutable(nullptr, 0, &kCFTypeDictionaryKeyCallBacks,
                                            &kCFTypeDictionaryValueCallBacks);
        /* CVPixelAspectRatio dict */
        dict_set_i32(par, CFSTR ("HorizontalSpacing"), 0);
        dict_set_i32(par, CFSTR ("VerticalSpacing"), 0);

        /* SampleDescriptionExtensionAtoms dict */
        switch (mVideoCodecType) {
            case kCMVideoCodecType_H264:
                dict_set_data(atoms, CFSTR ("avcC"), (uint8_t *) pData, size);
                break;

            case kCMVideoCodecType_HEVC:
                dict_set_data(atoms, CFSTR ("hvcC"), (uint8_t *) pData, size);
                break;

            case kCMVideoCodecType_MPEG4Video:
                dict_set_data(atoms, CFSTR ("esds"), (uint8_t *) pData, size);
                break;

            default:
                break;
        }

        /* Extensions dict */
        dict_set_string(extensions, CFSTR ("CVImageBufferChromaLocationBottomField"), "left");
        dict_set_string(extensions, CFSTR ("CVImageBufferChromaLocationTopField"), "left");
        dict_set_boolean(extensions, CFSTR("FullRangeVideo"), FALSE);
        dict_set_object(extensions, CFSTR ("CVPixelAspectRatio"), (CFTypeRef *) par);
        dict_set_object(extensions, CFSTR ("SampleDescriptionExtensionAtoms"), (CFTypeRef *) atoms);

        if (width <= 0 || height <= 0) {
            parserInfo info{0};
            Stream_meta *meta = ((Stream_meta *) (*(mPInMeta)));
            ret = parser_extradata(pData, size, &info, meta->codec);

            if (ret < 0) {
                CFRelease(extensions);
                CFRelease(atoms);
                CFRelease(par);
                return -EINVAL;
            }

            width = info.width;
            height = info.height;
        }

        status = CMVideoFormatDescriptionCreate(nullptr, mVideoCodecType, width, height, extensions, &cm_fmt_desc);
        CFRelease(extensions);
        CFRelease(atoms);
        CFRelease(par);

        if (status == 0) {
            return 0;
        } else {
            return -EINVAL;
        }
    }

#if 0
    int AFVTBDecoder::createVideoFormatDesc(const uint8_t *pData, int size, int width, int height, CFDictionaryRef &decoder_spec,
                                            CMFormatDescriptionRef &cm_fmt_desc)
    {
        OSStatus status = -1;

        if (pData == nullptr || size == 0) {
            return -EINVAL;
        }

        parserInfo info{};
        decoder_spec = videotoolbox_decoder_config_create(mVideoCodecType, pData, size, &info);

        if (!width || !height) {
            width = info.width;
            height = info.height;
        }

        // TODO: add width and height
        status = CMVideoFormatDescriptionCreate(kCFAllocatorDefault,
                                                mVideoCodecType,
                                                width,
                                                height,
                                                decoder_spec, // Dictionary of extension
                                                &cm_fmt_desc);

        if (!cm_fmt_desc) {
            if (decoder_spec) {
                CFRelease(decoder_spec);
            }

            return -1;
        }

        return 0;
    }
#endif

    int AFVTBDecoder::enqueue_decoder(unique_ptr<IAFPacket> &pPacket)
    {
        if (mResignActive) {
            close_decoder();
            std::lock_guard<std::mutex> lock(mActiveStatusMutex);
            mResignActive = false;
        }

        if (!mActive) {
            return -EAGAIN;
        }

        if (mVTDecompressSessionRef == nullptr) {
            gen_recovering_queue();
            init_decoder_internal();
        }

        int64_t startDecodeTime = af_getsteady_ms();

        while (!mRecoveringQueue.empty()) {
            int ret = enqueue_decoder_internal(mRecoveringQueue.front());

            if (ret != -EAGAIN) {
                mRecoveringQueue.pop();
            } else {
                return -EAGAIN;
            }

            if (!mRunning || !mActive) {
                return -EAGAIN;
            }

            if (af_getsteady_ms() - startDecodeTime > 200) {
                return -EAGAIN;
            }
        }

        if (pPacket == nullptr) {
            VTDecompressionSessionWaitForAsynchronousFrames(mVTDecompressSessionRef);
            flushReorderQueue();
            return STATUS_EOS;
        }

        return enqueue_decoder_internal(pPacket);
    }

    int AFVTBDecoder::process_extra_data(IAFPacket *pPacket)
    {
        if (pPacket == nullptr || pPacket->getInfo().extra_data == nullptr) {
            return 0;
        }

        Stream_meta *meta = ((Stream_meta *) (*(mPInMeta)));

        if (meta->extradata_size == pPacket->getInfo().extra_data_size) {
            if (memcmp(pPacket->getInfo().extra_data, meta->extradata, meta->extradata_size) == 0) {
                return 0;
            }
        }

        if (meta->extradata) {
            free(meta->extradata);
        }

        meta->extradata = static_cast<uint8_t *>(av_malloc(meta->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE));
        meta->extradata_size = pPacket->getInfo().extra_data_size;
        memcpy(meta->extradata, pPacket->getInfo().extra_data, pPacket->getInfo().extra_data_size);
        unique_ptr<bitStreamParser> parser = unique_ptr<bitStreamParser>(new bitStreamParser());
        int ret = parser->init(meta);
        parser->parser(pPacket->getData(), pPacket->getSize());
        parser->getPictureSize(meta->width, meta->height);

        if (mParser) {
            mParser = move(parser);
        }

        CM_NULLABLE CMVideoFormatDescriptionRef videoFormatDesRef{nullptr};
        CM_NULLABLE CFDictionaryRef decoder_spec{nullptr};
        int rv = createVideoFormatDesc(pPacket->getInfo().extra_data, pPacket->getInfo().extra_data_size, 0, 0, decoder_spec,
                                       videoFormatDesRef);

        if (rv == 0) {
            if (!VTDecompressionSessionCanAcceptFormatDescription(mVTDecompressSessionRef, videoFormatDesRef)) {
                flushReorderQueue();
                close_decoder();
                meta = ((Stream_meta *) (*(mPInMeta)));
                delete[] meta->extradata;
                meta->extradata = new uint8_t[pPacket->getInfo().extra_data_size];
                meta->extradata_size = pPacket->getInfo().extra_data_size;
                memcpy(meta->extradata, pPacket->getInfo().extra_data, pPacket->getInfo().extra_data_size);
                mVideoFormatDesRef = videoFormatDesRef;
                mDecoder_spec = decoder_spec;
                rv = init_decoder_internal();

                if (rv < 0) {
                    return -EINVAL;
                }
            } else {
                if (videoFormatDesRef) {
                    CFRelease(videoFormatDesRef);
                }
            }

            mPocDelta = 2;
        } else {
            if (videoFormatDesRef) {
                CFRelease(videoFormatDesRef);
            }

            if (decoder_spec) {
                CFRelease(decoder_spec);
            }

            return -EINVAL;
        }

        return 0;
    }

    int AFVTBDecoder::enqueue_decoder_internal(unique_ptr<IAFPacket> &pPacket)
    {
        //   AF_LOGD("mInputQueue size is %d\n", mInputQueue.size());
//        if (mOutputQueue.size() > maxOutQueueSize) {
//            //       AF_TRACE;
//            return -EAGAIN;
//        }
        if (pPacket->getInfo().flags & AF_PKT_FLAG_KEY) {
            mThrowPacket = false;
        }

        if (mThrowPacket) {
            AF_LOGE("IOS8VT: throw frame");
            return 0;
        }

        if (!mActive) {
            AF_LOGD("ios bg decoder inactive return resign active");
            return -EAGAIN;
        }

        if (pPacket->getInfo().extra_data) {
            int ret = process_extra_data(pPacket.get());

            if (ret < 0) {
                return ret;
            }
        }

        CMBlockBufferRef newBuffer = nullptr;

        if (CMBlockBufferCreateWithMemoryBlock(kCFAllocatorDefault, pPacket->getData(), pPacket->getSize(), kCFAllocatorNull,
                                               nullptr, 0, pPacket->getSize(), 0, &newBuffer) != 0) {
            AF_LOGE("failed to create mblockbuffer");
            return -EINVAL;
        }

        CMSampleTimingInfo timeInfo;
        timeInfo.presentationTimeStamp = CMTimeMake(pPacket->getInfo().pts, 1000000);
        timeInfo.duration = CMTimeMake(pPacket->getInfo().duration, 1000000);
        timeInfo.decodeTimeStamp = CMTimeMake(pPacket->getInfo().dts, 1000000);
        CMSampleBufferRef sampleBuffer = nullptr;

        if (0 != CMSampleBufferCreate(kCFAllocatorDefault, newBuffer, TRUE, nullptr, nullptr, mVideoFormatDesRef,
                                      1, 1, &timeInfo, 0, nullptr, &sampleBuffer)) {
            CFRelease(newBuffer); // add by yager
            AF_LOGE("failed to CMSampleBufferCreate");
            return -EINVAL;
        }

        VTDecodeInfoFlags infoFlags;
        VTDecodeFrameFlags flags = 0;

        if (pPacket->getDiscard()) {
            flags |= kVTDecodeFrame_DoNotOutputFrame;
        }

        IAFPacket *packet = pPacket.release();
        int rv = VTDecompressionSessionDecodeFrame(mVTDecompressSessionRef, sampleBuffer, flags, packet, &infoFlags);
        CFRelease(newBuffer);
        CFRelease(sampleBuffer);

        if (rv == kVTInvalidSessionErr) {
            AF_LOGW("kVTInvalidSessionErr\n");
            pPacket.reset(packet);
            close_decoder();
            return -EAGAIN;
        }

        if (rv != noErr) {
            // TODO: enqueue error
            /*
             * VTDecompressionSessionDecodeFrame WILL HAVE callback when return error,
             * so can't push it to queue
             */
            //     push_to_recovery_queue(unique_ptr<IAFPacket>(packet));
        }

        if (rv != noErr && !(infoFlags & kVTDecodeInfo_FrameDropped)) {
            if (rv == kVTVideoDecoderBadDataErr) {
                mThrowPacket = true;
                AF_LOGE("IOSVT: decode failed kVTVideoDecoderBadDataErr");
                // bad data error, return warning.
                return -EINVAL;
            } else {
                AF_LOGE("IOSVT: decode failed status=%d", (int) rv);
                return -EINVAL;
            }
        }

        return 0;
    }

    void AFVTBDecoder::onDecoded(IAFPacket *packet, std::unique_ptr<PBAFFrame> frame, OSStatus status)
    {
        if (packet == nullptr) {
            return;
        }

        if (status != noErr) {
            AF_LOGW("AFVTBDecoder decoder error %d\n", status);
            enqueueError(status, packet->getInfo().pts);
        }

        bool keyFrame = (packet->getInfo().flags & AF_PKT_FLAG_KEY);
        int64_t mapKey = packet->getInfo().pts;

        // must parser poc
        if (mBUsePoc && mPocErrorCount < MAX_POC_ERROR) {
            assert(mParser != nullptr);
            mParser->parser(packet->getData(), packet->getSize());
            int poc = mParser->getPOC();

            if (poc < 0 || (poc == 0 && !keyFrame)) {
                AF_LOGI("error poc is %d\n", poc);
                if (++mPocErrorCount >= MAX_POC_ERROR) {
                    AF_LOGE("too much poc error, disable reorder use poc\n");
                }
                push_to_recovery_queue(unique_ptr<IAFPacket>(packet));
                return;
            }

            if (poc == 1) {
                mPocDelta = 1;
            }

            mapKey = poc;
        }

        if (frame == nullptr) {
            push_to_recovery_queue(unique_ptr<IAFPacket>(packet));

            if (mBUsePoc) {
                mOutputPoc = mapKey;
            }

            return;
        }

        if (mVideoCodecType != kCMVideoCodecType_HEVC && keyFrame && mPocErrorCount < MAX_POC_ERROR) {
            flushReorderQueue();

            if (mVTOutFmt == AF_PIX_FMT_YUV420P) {
                auto *avframe = (AVAFFrame *) (*frame);
                std::unique_lock<std::mutex> uMutex(mReorderMutex);
                mReorderedQueue.push(unique_ptr<IAFFrame>(avframe));
            } else {
                std::unique_lock<std::mutex> uMutex(mReorderMutex);
                mReorderedQueue.push(move(frame));
            }

            mOutputPoc = 0;
        } else {
            std::unique_lock<std::mutex> uMutex(mReorderMutex);
            mReorderFrameMap[mapKey] = move(frame);
        }

        push_to_recovery_queue(unique_ptr<IAFPacket>(packet));
    }

    void AFVTBDecoder::decompressionOutputCallback(void *CM_NULLABLE decompressionOutputRefCon,
            void *CM_NULLABLE sourceFrameRefCon,
            OSStatus status,
            VTDecodeInfoFlags infoFlags,
            CM_NULLABLE CVImageBufferRef imageBuffer,
            CMTime presentationTimeStamp,
            CMTime presentationDuration)
    {
        auto *decoder = static_cast<AFVTBDecoder *> (decompressionOutputRefCon);
        IAFPacket *packet = static_cast<IAFPacket *>(sourceFrameRefCon);
        unique_ptr<PBAFFrame> pbafFrame{};

        if (infoFlags & kVTDecodeInfo_FrameDropped) {
            AF_LOGI("kVTDecodeInfo_FrameDropped");
        }

        // TODO: why status is 1 when recovering

        if (status != noErr || imageBuffer == nullptr || packet->getDiscard()) {
            // TODO: enqueue error
            return decoder->onDecoded(packet, nullptr, status);
        }

        int64_t duration = presentationDuration.value * (1000000.0 / presentationDuration.timescale);
        int64_t pts = presentationTimeStamp.value * (1000000.0 / presentationTimeStamp.timescale);
        pbafFrame = unique_ptr<PBAFFrame>(new PBAFFrame(imageBuffer, pts, duration));
        return decoder->onDecoded(packet, move(pbafFrame), status);
    }

    int AFVTBDecoder::dequeue_decoder(unique_ptr<IAFFrame> &pFrame)
    {
        //   AF_LOGD("mOutputQueue size is %d\n", mOutputQueue.size());
        std::unique_lock<std::mutex> uMutex(mReorderMutex);

        if (!mReorderedQueue.empty()) {
            pFrame = move(mReorderedQueue.front());
            mReorderedQueue.pop();
            return 0;
        }

        if (mBUsePoc) {
            if (mReorderFrameMap.empty()) {
                return -EAGAIN;
            }

            int64_t poc = (*mReorderFrameMap.begin()).first;

            if (poc < 0 || mOutputPoc < 0
                    || (mReorderFrameMap.size() >= IOS_OUTPUT_CACHE_FOR_B_FRAMES)
                    || (poc - mOutputPoc == mPocDelta)) {
                mOutputPoc = poc;
            } else {
                return -EAGAIN;
            }
        } else if (mReorderFrameMap.size() < IOS_OUTPUT_CACHE_FOR_B_FRAMES) {
            return -EAGAIN;
        }

        if (mVTOutFmt == AF_PIX_FMT_YUV420P) {
            PBAFFrame *pbafFrame = (*(mReorderFrameMap.begin())).second.get();
            auto *frame = (AVAFFrame *) (*pbafFrame);
            pFrame = unique_ptr<IAFFrame>(frame);
        } else {
            pFrame = move(*(mReorderFrameMap.begin())).second;
        }

        mReorderFrameMap.erase(mReorderFrameMap.begin());
        return 0;
    }

    void AFVTBDecoder::gen_recovering_queue()
    {
        // push the RecoveringQueue to RecoveryQueue
        while (!mRecoveringQueue.empty()) {
            mRecoveryQueue.push(move(mRecoveringQueue.front()));
            mRecoveringQueue.pop();
        }

        assert(mRecoveringQueue.empty());

        while (!mRecoveryQueue.empty()) {
            mRecoveringQueue.push(move(mRecoveryQueue.front()));
            mRecoveryQueue.pop();
        }
    }

    void AFVTBDecoder::push_to_recovery_queue(std::unique_ptr<IAFPacket> pPacket)
    {
        pPacket->setDiscard(true);
        //      std::lock_guard<std::mutex> lock(mActiveStatusMutex);

        if (pPacket->getInfo().flags & AF_PKT_FLAG_KEY) {
            while (!mRecoveryQueue.empty()) {
                mRecoveryQueue.pop();
            }
        }

        mRecoveryQueue.push(move(pPacket));
    }

    void AFVTBDecoder::flushReorderQueue()
    {
        std::unique_lock<std::mutex> uMutex(mReorderMutex);

        while (!mReorderFrameMap.empty()) {
            if (mVTOutFmt == AF_PIX_FMT_YUV420P) {
                PBAFFrame *pbafFrame = (*(mReorderFrameMap.begin())).second.get();
                auto *frame = (AVAFFrame *) (*pbafFrame);
                mReorderedQueue.push(unique_ptr<IAFFrame>(frame));
            } else {
                mReorderedQueue.push(move(*(mReorderFrameMap.begin())).second);
            }

            mReorderFrameMap.erase(mReorderFrameMap.begin());
        }
    }

    void AFVTBDecoder::flush_decoder()
    {
        if (mVTDecompressSessionRef && mInputCount > 0 && mActive) {
            VTDecompressionSessionWaitForAsynchronousFrames(mVTDecompressSessionRef);
            mInputCount = 0;
        }

        {
            std::unique_lock<std::mutex> uMutex(mReorderMutex);
            mReorderFrameMap.clear();
        }

        std::lock_guard<std::mutex> lock(mActiveStatusMutex);

        while (!mRecoveringQueue.empty()) {
            mRecoveringQueue.pop();
        }

        while (!mRecoveryQueue.empty()) {
            mRecoveryQueue.pop();
        }

        while (!mReorderedQueue.empty()) {
            mReorderedQueue.pop();
        }
        mPocErrorCount = 0;
    }

    void AFVTBDecoder::AppWillResignActive()
    {
        std::lock_guard<std::mutex> lock(mActiveStatusMutex);
        AF_LOGD("ios bg decoder appWillResignActive");
        mActive = false;
        mResignActive = true;
//        if (mDecodedHandler) {
//            mDecodedHandler->OnDecodedMsgHandle(CICADA_VDEC_WARNING_IOS_RESIGN_ACTIVE);
//        }
    }

    void AFVTBDecoder::AppDidBecomeActive()
    {
        std::lock_guard<std::mutex> lock(mActiveStatusMutex);
        AF_LOGD("ios bg decoder appDidBecomeActive");
        mThrowPacket = true;
        mActive = true;
    }

    int AFVTBDecoder::get_decoder_recover_size()
    {
        std::lock_guard<std::mutex> lock(mActiveStatusMutex);
        return mRecoveryQueue.size();
    }

} // namespace
