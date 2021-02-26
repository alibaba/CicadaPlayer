//
// Created by pingkai on 2021/1/26.
//

#define LOG_TAG "pixelBufferConvertor"
#include "pixelBufferConvertor.h"
#include <base/media/AVAFPacket.h>
#include <utils/frame_work_log.h>

using namespace Cicada;
static int copy_avframe_to_pixel_buffer(const AVFrame *frame, CVPixelBufferRef cv_img, const size_t *plane_strides,
                                        const size_t *plane_rows)
{
    int i, j;
    size_t plane_count;
    int status;
    int rows;
    int src_stride;
    int dst_stride;
    uint8_t *src_addr;
    uint8_t *dst_addr;
    size_t copy_bytes;

    status = CVPixelBufferLockBaseAddress(cv_img, 0);
    if (status) {
        AF_LOGE("Error: Could not lock base address of CVPixelBuffer: %d.\n", status);
    }

    if (CVPixelBufferIsPlanar(cv_img)) {
        plane_count = CVPixelBufferGetPlaneCount(cv_img);
        for (i = 0; frame->data[i]; i++) {
            if (i == plane_count) {
                CVPixelBufferUnlockBaseAddress(cv_img, 0);
                AF_LOGE("Error: different number of planes in AVFrame and CVPixelBuffer.\n");

                return AVERROR_EXTERNAL;
            }

            dst_addr = (uint8_t *) CVPixelBufferGetBaseAddressOfPlane(cv_img, i);
            src_addr = (uint8_t *) frame->data[i];
            dst_stride = CVPixelBufferGetBytesPerRowOfPlane(cv_img, i);
            src_stride = plane_strides[i];
            rows = plane_rows[i];

            if (dst_stride == src_stride) {
                memcpy(dst_addr, src_addr, src_stride * rows);
            } else {
                copy_bytes = dst_stride < src_stride ? dst_stride : src_stride;

                for (j = 0; j < rows; j++) {
                    memcpy(dst_addr + j * dst_stride, src_addr + j * src_stride, copy_bytes);
                }
            }
        }
    } else {
        if (frame->data[1]) {
            CVPixelBufferUnlockBaseAddress(cv_img, 0);
            AF_LOGE("Error: different number of planes in AVFrame and non-planar CVPixelBuffer.\n");

            return AVERROR_EXTERNAL;
        }

        dst_addr = (uint8_t *) CVPixelBufferGetBaseAddress(cv_img);
        src_addr = (uint8_t *) frame->data[0];
        dst_stride = CVPixelBufferGetBytesPerRow(cv_img);
        src_stride = plane_strides[0];
        rows = plane_rows[0];

        if (dst_stride == src_stride) {
            memcpy(dst_addr, src_addr, src_stride * rows);
        } else {
            copy_bytes = dst_stride < src_stride ? dst_stride : src_stride;

            for (j = 0; j < rows; j++) {
                memcpy(dst_addr + j * dst_stride, src_addr + j * src_stride, copy_bytes);
            }
        }
    }

    status = CVPixelBufferUnlockBaseAddress(cv_img, 0);
    if (status) {
        AF_LOGE("Error: Could not unlock CVPixelBuffer base address: %d.\n", status);
        return AVERROR_EXTERNAL;
    }

    return 0;
}

static int get_cv_pixel_format(enum AVPixelFormat fmt, enum AVColorRange range, int *av_pixel_format, int *range_guessed)
{
    if (range_guessed) *range_guessed = range != AVCOL_RANGE_MPEG && range != AVCOL_RANGE_JPEG;

    //MPEG range is used when no range is set
    if (fmt == AV_PIX_FMT_NV12) {
        *av_pixel_format = range == AVCOL_RANGE_JPEG ? kCVPixelFormatType_420YpCbCr8BiPlanarFullRange
                                                     : kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;
    } else if (fmt == AV_PIX_FMT_YUV420P) {
        *av_pixel_format = range == AVCOL_RANGE_JPEG ? kCVPixelFormatType_420YpCbCr8PlanarFullRange : kCVPixelFormatType_420YpCbCr8Planar;
    } else if (fmt == AV_PIX_FMT_P010LE) {
        *av_pixel_format = range == AVCOL_RANGE_JPEG ? kCVPixelFormatType_420YpCbCr10BiPlanarFullRange
                                                     : kCVPixelFormatType_420YpCbCr10BiPlanarVideoRange;
        *av_pixel_format = kCVPixelFormatType_420YpCbCr10BiPlanarVideoRange;
    } else {
        return -(EINVAL);
    }

    return 0;
}

static int get_cv_pixel_info(const AVFrame *frame, int *color, int *plane_count, size_t *widths, size_t *heights, size_t *strides,
                             size_t *contiguous_buf_size)
{
    int av_format = frame->format;
    int av_color_range = frame->color_range;
    int i;
    int range_guessed;
    int status;

    status = get_cv_pixel_format(static_cast<AVPixelFormat>(av_format), static_cast<AVColorRange>(av_color_range), color, &range_guessed);
    if (status) {
        AF_LOGE("Could not get pixel format for color format");
        return -(EINVAL);
    }

    switch (av_format) {
        case AV_PIX_FMT_NV12:
            *plane_count = 2;

            widths[0] = frame->width;
            heights[0] = frame->height;
            strides[0] = frame ? frame->linesize[0] : frame->width;

            widths[1] = (frame->width + 1) / 2;
            heights[1] = (frame->height + 1) / 2;
            strides[1] = frame ? frame->linesize[1] : (frame->width + 1) & -2;
            break;

        case AV_PIX_FMT_YUV420P:
            *plane_count = 3;

            widths[0] = frame->width;
            heights[0] = frame->height;
            strides[0] = frame ? frame->linesize[0] : frame->width;

            widths[1] = (frame->width + 1) / 2;
            heights[1] = (frame->height + 1) / 2;
            strides[1] = frame ? frame->linesize[1] : (frame->width + 1) / 2;

            widths[2] = (frame->width + 1) / 2;
            heights[2] = (frame->height + 1) / 2;
            strides[2] = frame ? frame->linesize[2] : (frame->width + 1) / 2;
            break;

        case AV_PIX_FMT_P010LE:
            *plane_count = 2;
            widths[0] = frame->width;
            heights[0] = frame->height;
            strides[0] = frame ? frame->linesize[0] : (frame->width * 2 + 63) & -64;

            widths[1] = (frame->width + 1) / 2;
            heights[1] = (frame->height + 1) / 2;
            strides[1] = frame ? frame->linesize[1] : ((frame->width + 1) / 2 + 63) & -64;
            break;

        default:
            AF_LOGE("Could not get frame format info for color %d range %d.\n", av_format, av_color_range);
            return -(EINVAL);
    }

    *contiguous_buf_size = 0;
    for (i = 0; i < *plane_count; i++) {
        if (i < *plane_count - 1 && frame->data[i] + strides[i] * heights[i] != frame->data[i + 1]) {
            *contiguous_buf_size = 0;
            break;
        }

        *contiguous_buf_size += strides[i] * heights[i];
    }

    return 0;
}

void cfdict_set_int32(CFMutableDictionaryRef dict, CFStringRef key, int value)
{
    CFNumberRef number = CFNumberCreate(NULL, kCFNumberSInt32Type, &value);
    CFDictionarySetValue(dict, key, number);
    CFRelease(number);
}
CFMutableDictionaryRef cfdict_create(CFIndex capacity)
{
    return CFDictionaryCreateMutable(kCFAllocatorDefault, capacity, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
}

CVPixelBufferPoolRef cvpxpool_create(const IAFFrame::videoInfo &src, unsigned count)
{
    int cvpx_format;
    switch (src.format) {
        case AF_PIX_FMT_UYVY422:
            cvpx_format = kCVPixelFormatType_422YpCbCr8;
            break;
        case AF_PIX_FMT_NV12:
            if (src.colorRange == AVCOL_RANGE_JPEG) {
                cvpx_format = kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;
            } else
                cvpx_format = kCVPixelFormatType_420YpCbCr8BiPlanarFullRange;
            break;
        case AF_PIX_FMT_YUV420P:
            cvpx_format = kCVPixelFormatType_420YpCbCr8Planar;
            break;
            //        case AV_PIX_FMT_BGR0:
            //            cvpx_format = kCVPixelFormatType_32BGRA;
            //            break;
            //        case AV_PIX_FMT_P010:
            //            cvpx_format = 'x420'; /* kCVPixelFormatType_420YpCbCr10BiPlanarVideoRange */
            //            break;
        default:
            return nullptr;
    }

    /* destination pixel buffer attributes */
    CFMutableDictionaryRef cvpx_attrs_dict = cfdict_create(5);
    if (cvpx_attrs_dict == nullptr) return nullptr;
    CFMutableDictionaryRef pool_dict = cfdict_create(2);
    if (pool_dict == nullptr) {
        CFRelease(cvpx_attrs_dict);
        return nullptr;
    }

    CFMutableDictionaryRef io_dict = cfdict_create(0);
    if (io_dict == nullptr) {
        CFRelease(cvpx_attrs_dict);
        CFRelease(pool_dict);
        return nullptr;
    }
    CFDictionarySetValue(cvpx_attrs_dict, kCVPixelBufferIOSurfacePropertiesKey, io_dict);
    CFRelease(io_dict);

    cfdict_set_int32(cvpx_attrs_dict, kCVPixelBufferPixelFormatTypeKey, cvpx_format);
    cfdict_set_int32(cvpx_attrs_dict, kCVPixelBufferWidthKey, src.width);
    cfdict_set_int32(cvpx_attrs_dict, kCVPixelBufferHeightKey, src.height);
    /* Required by CIFilter to render IOSurface */
    cfdict_set_int32(cvpx_attrs_dict, kCVPixelBufferBytesPerRowAlignmentKey, 16);

    cfdict_set_int32(pool_dict, kCVPixelBufferPoolMinimumBufferCountKey, count);
    cfdict_set_int32(pool_dict, kCVPixelBufferPoolMaximumBufferAgeKey, 0);

    CVPixelBufferPoolRef pool;
    CVReturn err = CVPixelBufferPoolCreate(nullptr, pool_dict, cvpx_attrs_dict, &pool);
    CFRelease(pool_dict);
    CFRelease(cvpx_attrs_dict);
    if (err != kCVReturnSuccess) {
        return nullptr;
    }

    CVPixelBufferRef cvpxs[count];
    for (unsigned i = 0; i < count; ++i) {
        err = CVPixelBufferPoolCreatePixelBuffer(nullptr, pool, &cvpxs[i]);
        if (err != kCVReturnSuccess) {
            CVPixelBufferPoolRelease(pool);
            pool = nullptr;
            count = i;
            break;
        }
    }
    for (unsigned i = 0; i < count; ++i) {
        CFRelease(cvpxs[i]);
    }

    return pool;
}

CVPixelBufferRef pixelBufferConvertor::avFrame2pixelBuffer(AVFrame *frame)
{
    int plane_count;
    int color;
    size_t widths[AV_NUM_DATA_POINTERS];
    size_t heights[AV_NUM_DATA_POINTERS];
    size_t strides[AV_NUM_DATA_POINTERS];
    int status;
    size_t contiguous_buf_size;
    memset(widths, 0, sizeof(widths));
    memset(heights, 0, sizeof(heights));
    memset(strides, 0, sizeof(strides));
    status = get_cv_pixel_info(frame, &color, &plane_count, widths, heights, strides, &contiguous_buf_size);
    if (status) {
        AF_LOGE("Error: Cannot convert format %d color_range %d: %d\n", frame->format, frame->color_range, status);
        return nullptr;
    }

    CVPixelBufferRef pixelBuffer;
    status = CVPixelBufferPoolCreatePixelBuffer(nullptr, mPixBufPool, &pixelBuffer);

    if (status) {
        AF_LOGE("Could not create pixel buffer from pool: %d.\n", status);
        return nullptr;
    }

    status = copy_avframe_to_pixel_buffer(frame, pixelBuffer, strides, heights);
    if (status) {
        CFRelease(pixelBuffer);
        pixelBuffer = nullptr;
    }
    return pixelBuffer;
}
pixelBufferConvertor::pixelBufferConvertor()
{
    memset(&mVideoInfo, 0, sizeof(IAFFrame::videoInfo));
    mVideoInfo.format = -1;
}
pixelBufferConvertor::~pixelBufferConvertor()
{
    if (sws_ctx) {
        sws_freeContext(sws_ctx);
    }
    av_frame_free(&mOutFrame);
    if (mPixBufPool) {
        CVPixelBufferPoolRelease(mPixBufPool);
    }
}
static AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width, int height)
{
    AVFrame *picture;
    int ret;

    picture = av_frame_alloc();
    if (!picture) return nullptr;

    picture->format = pix_fmt;
    picture->width = width;
    picture->height = height;

    /* allocate the buffers for the frame data */
    ret = av_frame_get_buffer(picture, 32);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate frame data.\n");
        exit(1);
    }

    return picture;
}

int pixelBufferConvertor::init(const IAFFrame::videoInfo &src)
{
    // TODO: get the dst format
    enum AVPixelFormat dstFormat;
    IAFFrame::videoInfo dst = src;
    switch (src.format) {
        case AF_PIX_FMT_UYVY422:
        case AF_PIX_FMT_NV12:
        case AF_PIX_FMT_YUV420P:
            break;
        default:
            dst.format = AV_PIX_FMT_NV12;
    }


    if (mPixBufPool) {
        CVPixelBufferPoolRelease(mPixBufPool);
    }

    mPixBufPool = cvpxpool_create(dst, 3);

    if (mPixBufPool == nullptr) {
        return -EINVAL;
    }


    if (sws_ctx) {
        sws_freeContext(sws_ctx);
        sws_ctx = nullptr;
    }
    av_frame_free(&mOutFrame);

    if (src != dst) {

        sws_ctx = sws_getContext(src.width, src.height, static_cast<AVPixelFormat>(src.format), src.width, src.height, dstFormat,
                                 SWS_BILINEAR, nullptr, nullptr, nullptr);
        mOutFrame = alloc_picture(dstFormat, src.width, src.height);
    }
    return 0;
}

IAFFrame *pixelBufferConvertor::convert(IAFFrame *frame)
{
    auto *avAFFrame = dynamic_cast<AVAFFrame *>(frame);
    if (avAFFrame == nullptr) {
        return nullptr;
    }

    if (mVideoInfo != frame->getInfo().video) {
        int ret = init(frame->getInfo().video);
        if (ret < 0) {
            AF_LOGE("convert init error %d\n", ret);
            return nullptr;
        }
        mVideoInfo = frame->getInfo().video;
    }
    auto *avFrame = (AVFrame *) (*avAFFrame);

    if (sws_ctx) {
        sws_scale(sws_ctx, avFrame->data, avFrame->linesize, 0, avFrame->height, mOutFrame->data, mOutFrame->linesize);
        avFrame = mOutFrame;
    }

    CVPixelBufferRef pixelBuffer = avFrame2pixelBuffer(avFrame);
    if (pixelBuffer == nullptr) {
        return nullptr;
    }
    VideoColorInfo colorInfo;
    colorInfo.chroma_location = static_cast<AFChromaLocation>(avFrame->chroma_location);
    colorInfo.color_primaries = static_cast<AFColorPrimaries>(avFrame->color_primaries);
    colorInfo.color_range = static_cast<AFColorRange>(avFrame->color_range);
    colorInfo.color_space = static_cast<AFColorSpace>(avFrame->colorspace);
    colorInfo.color_trc = static_cast<AFColorTransferCharacteristic>(avFrame->color_trc);

    UpdateColorInfo(colorInfo, pixelBuffer);
    auto *pBFrame = new PBAFFrame(pixelBuffer, frame->getInfo().pts, frame->getInfo().duration);
    CFRelease(pixelBuffer);
    return pBFrame;
}
void pixelBufferConvertor::UpdateColorInfo(const VideoColorInfo &info, CVPixelBufferRef pixelBuffer)
{
    CFStringRef value;
    switch (info.color_primaries) {
        case AFCOL_PRI_BT709:
            value = kCVImageBufferColorPrimaries_ITU_R_709_2;
            break;
        case AFCOL_PRI_SMPTE170M:
            value = kCVImageBufferColorPrimaries_SMPTE_C;
            break;
        case AFCOL_PRI_BT2020:
            value = kCVImageBufferColorPrimaries_ITU_R_2020;
            break;
        default:
            value = nullptr;
            break;
    }
    if (value) {
        CVBufferSetAttachment(pixelBuffer, kCVImageBufferColorPrimariesKey, value, kCVAttachmentMode_ShouldPropagate);
    }
    value = nullptr;
    switch (info.color_trc) {
        case AFCOL_TRC_BT709:
        case AFCOL_TRC_SMPTE170M:
            value = kCVImageBufferTransferFunction_ITU_R_709_2;
            break;
        case AFCOL_TRC_BT2020_10:
            value = kCVImageBufferTransferFunction_ITU_R_2020;
            break;
        case AFCOL_TRC_SMPTE2084:
#if TARGET_OS_IPHONE
            if (__builtin_available(iOS 11.0, *))
#else
            if (__builtin_available(macOS 10.13, *))
#endif
            {
                value = kCVImageBufferTransferFunction_SMPTE_ST_2084_PQ;
            }
            break;
        case AFCOL_TRC_SMPTE428:
#if TARGET_OS_IPHONE
            if (__builtin_available(iOS 10.0, *))
#else
            if (__builtin_available(macOS 10.12, *))
#endif
            {
                value = kCVImageBufferTransferFunction_SMPTE_ST_428_1;
            }
            break;
        case AFCOL_TRC_ARIB_STD_B67:
#if TARGET_OS_IPHONE
            if (__builtin_available(iOS 11.0, *))
#else
            if (__builtin_available(macOS 10.13, *))
#endif
            {
                value = kCVImageBufferTransferFunction_ITU_R_2100_HLG;
            }
            break;
        default:
            value = nullptr;
            break;
    }
    if (value) {
        CVBufferSetAttachment(pixelBuffer, kCVImageBufferTransferFunctionKey, value, kCVAttachmentMode_ShouldPropagate);
    }

    switch (info.color_space) {
        case AFCOL_SPC_BT709:
            value = kCGColorSpaceITUR_709;
            CVBufferSetAttachment(pixelBuffer, kCVImageBufferYCbCrMatrixKey, kCVImageBufferYCbCrMatrix_ITU_R_709_2,
                                  kCVAttachmentMode_ShouldPropagate);
            break;
        case AFCOL_SPC_BT2020_NCL:
            value = kCGColorSpaceITUR_2020;
            CVBufferSetAttachment(pixelBuffer, kCVImageBufferYCbCrMatrixKey, kCVImageBufferYCbCrMatrix_ITU_R_2020,
                                  kCVAttachmentMode_ShouldPropagate);
            break;
        case AFCOL_SPC_SMPTE170M:
            value = kCGColorSpaceSRGB;
            CVBufferSetAttachment(pixelBuffer, kCVImageBufferYCbCrMatrixKey, kCVImageBufferYCbCrMatrix_ITU_R_601_4,
                                  kCVAttachmentMode_ShouldPropagate);
            break;
        default:
            value = nullptr;
            break;
    }
    _Nullable CGColorSpaceRef m_ColorSpace{nullptr};
    if (value) {
        m_ColorSpace = CGColorSpaceCreateWithName(value);
    }
    if (m_ColorSpace != nullptr) {
        CVBufferSetAttachment(pixelBuffer, kCVImageBufferCGColorSpaceKey, m_ColorSpace, kCVAttachmentMode_ShouldPropagate);
    }
    CGColorSpaceRelease(m_ColorSpace);
}
