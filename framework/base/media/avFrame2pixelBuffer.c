//
// Created by moqi on 2020/6/5.
//

#include "avFrame2pixelBuffer.h"
#include <utils/frame_work_log.h>
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
                av_log(NULL, AV_LOG_ERROR, "Error: different number of planes in AVFrame and CVPixelBuffer.\n");

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
            av_log(NULL, AV_LOG_ERROR, "Error: different number of planes in AVFrame and non-planar CVPixelBuffer.\n");

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
        return AVERROR(EINVAL);
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

    status = get_cv_pixel_format(av_format, av_color_range, color, &range_guessed);
    if (status) {
        AF_LOGE("Could not get pixel format for color format");
        return AVERROR(EINVAL);
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
            av_log(NULL, AV_LOG_ERROR, "Could not get frame format info for color %d range %d.\n", av_format, av_color_range);

            return AVERROR(EINVAL);
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

static AVFrame *yuv420p2nv12(AVFrame *frame)
{
    int x, y;
    AVFrame *outFrame = av_frame_alloc();
    outFrame->format = AV_PIX_FMT_NV12;
    outFrame->width = frame->width;
    outFrame->height = frame->height;

    int ret = av_frame_get_buffer(outFrame, 32);
    if (ret < 0) {
        av_frame_free(&outFrame);
        return NULL;
    }
    ret = av_frame_make_writable(outFrame);
    if (ret < 0) {
        av_frame_free(&outFrame);
        return NULL;
    }
    if (frame->linesize[0] == frame->width) {
        memcpy(outFrame->data[0], frame->data[0], outFrame->width * frame->height);
    } else {
        for (y = 0; y < outFrame->height; ++y) {
            for (x = 0; x < outFrame->width; ++x) {
                outFrame->data[0][y * outFrame->linesize[0] + x] = frame->data[0][y * frame->linesize[0] + x];
            }
        }
    }

    for (y = 0; y < outFrame->height / 2; ++y) {
        for (x = 0; x < outFrame->width / 2; ++x) {
            outFrame->data[1][y * outFrame->linesize[1] + 2 * x] = frame->data[1][y * frame->linesize[1] + x];
            outFrame->data[1][y * outFrame->linesize[1] + 2 * x + 1] = frame->data[2][y * frame->linesize[2] + x];
        }
    }

    return outFrame;
}

CVPixelBufferRef avFrame2pixelBuffer(AVFrame *frame)
{
    int plane_count;
    int color;
    size_t widths[AV_NUM_DATA_POINTERS];
    size_t heights[AV_NUM_DATA_POINTERS];
    size_t strides[AV_NUM_DATA_POINTERS];
    int status;
    size_t contiguous_buf_size;
    CVPixelBufferPoolRef pix_buf_pool;
    memset(widths, 0, sizeof(widths));
    memset(heights, 0, sizeof(heights));
    memset(strides, 0, sizeof(strides));
    status = get_cv_pixel_info(frame, &color, &plane_count, widths, heights, strides, &contiguous_buf_size);
    if (status) {
        AF_LOGE("Error: Cannot convert format %d color_range %d: %d\n", frame->format, frame->color_range, status);

        return NULL;
    }

    CVPixelBufferRef pixelBuffer;
    OSType pixelFormat;
    if (frame->color_range == AVCOL_RANGE_MPEG) {
        pixelFormat = kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;
    } else
        pixelFormat = kCVPixelFormatType_420YpCbCr8BiPlanarFullRange;

    CFMutableDictionaryRef buffer_attributes;
    CFMutableDictionaryRef io_surface_properties;
    CFNumberRef cv_pix_fmt;
    CFNumberRef w;
    CFNumberRef h;
    w = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &frame->width);
    h = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &frame->height);

    cv_pix_fmt = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &pixelFormat);

    buffer_attributes = CFDictionaryCreateMutable(kCFAllocatorDefault, 4, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    io_surface_properties =
            CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    CFDictionarySetValue(buffer_attributes, kCVPixelBufferPixelFormatTypeKey, cv_pix_fmt);
    CFDictionarySetValue(buffer_attributes, kCVPixelBufferIOSurfacePropertiesKey, io_surface_properties);
    CFDictionarySetValue(buffer_attributes, kCVPixelBufferWidthKey, w);
    CFDictionarySetValue(buffer_attributes, kCVPixelBufferHeightKey, h);
#if TARGET_OS_IPHONE
    CFDictionarySetValue(buffer_attributes, kCVPixelBufferOpenGLESCompatibilityKey, kCFBooleanTrue);
#else
    CFDictionarySetValue(buffer_attributes, kCVPixelBufferIOSurfaceOpenGLTextureCompatibilityKey, kCFBooleanTrue);
#endif


    status = CVPixelBufferCreate(kCFAllocatorDefault, frame->width, frame->height, pixelFormat, buffer_attributes, &pixelBuffer);
    CFRelease(io_surface_properties);
    CFRelease(buffer_attributes);
    CFRelease(w);
    CFRelease(h);

    if (status) {
        AF_LOGE("Could not create pixel buffer from pool: %d.\n", status);
        return NULL;
    }

    AVFrame *nv12Frame = yuv420p2nv12(frame);

    status = copy_avframe_to_pixel_buffer(nv12Frame, pixelBuffer, strides, heights);
    av_frame_free(&nv12Frame);
    if (status) {
        CFRelease(pixelBuffer);
        pixelBuffer = NULL;
    }
    return pixelBuffer;
}