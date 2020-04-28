//
//  CicadaDef.h
//  CicadaSDK
//
//  Created by shiping.csp on 2018/11/16.
//  Copyright © 2018 com.alibaba.AliyunPlayer. All rights reserved.
//

#ifndef CicadaDef_h
#define CicadaDef_h

#import <Foundation/Foundation.h>

#if TARGET_OS_OSX
#import <AppKit/AppKit.h>
#define CicadaView NSView
#define CicadaImage NSImage
#elif TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#define CicadaView UIView
#define CicadaImage UIImage
#endif // TARGET_OS_OSX

#import "CicadaErrorCode.h"

/**
 * Convert playback URL before playback.
 *
 * For vid playback, try to convert the playback URL before playback
 *
 * @param   inputURL  [in]  input URL.
 * @param   srcFormat [in]  input format. should be "m3u8" "mp4", or "" if unknown.
 * @param   outputURL [out] output URL, convertURL function will malloc the memory, and user of PlayURLConverCallback need free it outside.
 *
 * @return  true if success.
 */
typedef bool (*PlayURLConverCallback)(const char* srcURL, const char* srcFormat, char** destURL);

typedef enum CicadaStatus: NSUInteger {
    /** @brief 空转，闲时，静态 */
    /**** @brief Idle */
    CicadaStatusIdle = 0,
    /** @brief 初始化完成 */
    /**** @brief Initialized */
    CicadaStatusInitialzed,
    /** @brief 准备完成 */
    /**** @brief Prepared */
    CicadaStatusPrepared,
    /** @brief 正在播放 */
    /**** @brief Playing */
    CicadaStatusStarted,
    /** @brief 播放暂停 */
    /**** @brief Paused */
    CicadaStatusPaused,
    /** @brief 播放停止 */
    /**** @brief Stopped */
    CicadaStatusStopped,
    /** @brief 播放完成 */
    /**** @brief Completed */
    CicadaStatusCompletion,
    /** @brief 播放错误 */
    /**** @brief Error */
    CicadaStatusError,
    /** @brief 界限值 */
    CicadaStatusMax,
} CicadaStatus;

 /**@brief 跳转模式，是否为精准跳转*/
 /****@brief Seeking mode: accurate seeking or inaccurate seeking.*/
typedef enum CicadaSeekMode: NSUInteger {
    CICADA_SEEKMODE_ACCURATE = 0x01,
    CICADA_SEEKMODE_INACCURATE = 0x10,
} CicadaSeekMode;

 /**@brief 渲染显示模式*/
 /****@brief Zoom mode*/
typedef enum CicadaScalingMode: NSUInteger {
    /**@brief 不保持比例平铺*/
    /****@brief Auto stretch to fit.*/
    CICADA_SCALINGMODE_SCALETOFILL,
    /**@brief 保持比例，黑边*/
    /****@brief Keep aspect ratio and add black borders.*/
    CICADA_SCALINGMODE_SCALEASPECTFIT,
    /**@brief 保持比例填充，需裁剪*/
    /****@brief Keep aspect ratio and crop.*/
    CICADA_SCALINGMODE_SCALEASPECTFILL,
} CicadaScalingMode;

/**@brief 旋转模式*/
/****@brief Rotate mode*/
typedef enum CicadaRotateMode: NSUInteger {
    CICADA_ROTATE_0 = 0,
    CICADA_ROTATE_90 = 90,
    CICADA_ROTATE_180 = 180,
    CICADA_ROTATE_270 = 270
} CicadaRotateMode;

/**@brief 镜像模式*/
/****@brief Mirroring mode*/
typedef enum CicadaMirrorMode: NSUInteger {
    CICADA_MIRRORMODE_NONE,
    CICADA_MIRRORMODE_HORIZONTAL,
    CICADA_MIRRORMODE_VERTICAL,
} CicadaMirrorMode;

/**@brief 播放器事件类型*/
/****@brief Player event type*/
typedef enum CicadaEventType: NSUInteger {
    /**@brief 准备完成事件*/
    /****@brief Preparation completion event*/
    CicadaEventPrepareDone,
    /**@brief 自动启播事件*/
    /****@brief Autoplay start event*/
    CicadaEventAutoPlayStart,
    /**@brief 首帧显示时间*/
    /****@brief First frame display event*/
    CicadaEventFirstRenderedStart,
    /**@brief 播放完成事件*/
    /****@brief Playback completion event*/
    CicadaEventCompletion,
    /**@brief 缓冲开始事件*/
    /****@brief Buffer start event*/
    CicadaEventLoadingStart,
    /**@brief 缓冲完成事件*/
    /****@brief Buffer completion event*/
    CicadaEventLoadingEnd,
    /**@brief 跳转完成事件*/
    /****@brief Seeking completion event*/
    CicadaEventSeekEnd,
    /**@brief 循环播放开始事件*/
    /****@brief Loop playback start event*/
    CicadaEventLoopingStart,
} CicadaEventType;

/**@brief 获取信息播放器的key*/
/****@brief The key to get property*/
typedef enum CicadaPropertyKey: NSUInteger {
    /**@brief Http的response信息
     * 返回的字符串是JSON数组，每个对象带response和type字段。type字段可以是url/video/audio/subtitle，根据流是否有相应Track返回。
     * 例如：[{"response":"response string","type":"url"},{"response":"","type":"video"}]
     */
    /****@brief Http response info
     * Return with JSON array，each object item include 'response'/'type' filed。'type' could be  url/video/audio/subtitle, depend on the stream whether have the tracks。
     * For example: [{"response":"response string","type":"url"},{"response":"","type":"video"}]
     */
    CICADA_KEY_RESPONSE_INFO = 0,

    /**@brief 主URL的连接信息
     * 返回的字符串是JSON对象，带url/ip/eagleID/cdnVia/cdncip/cdnsip等字段（如果解析不到则不添加）
     * 例如：{"url":"http://xxx","openCost":23,"ip":"11.111.111.11","cdnVia":"xxx","cdncip":"22.222.222.22","cdnsip":"xxx"}
     */
    /****@brief Major URL connect information
     * Return with JSON object，include sub fileds such as url/ip/eagleID/cdnVia/cdncip/cdnsip.
     * For example: {"url":"http://xxx","openCost":23,"ip":"11.111.111.11","cdnVia":"xxx","cdncip":"22.222.222.22","cdnsip":"xxx"}
     */
    CICADA_KEY_CONNECT_INFO  = 1,
} CicadaPropertyKey;

typedef enum CicadaOption: NSUInteger {
    /**
     * @brief 渲染的fps。类型为Float
     */
    /****
     * @brief render fps. Return value type is Float
     */
    CICADA_OPTION_RENDER_FPS = 0,
} CicadaOption;

typedef enum CicadaPlaybackType: NSUInteger {
    /**
     * @brief 播放音视频，默认值
     */
    /****
     * @brief play video & audio, default
     */
    CicadaPlaybackTypeALL = 0,
    /**
     * @brief 播放视频
     */
    /****
     * @brief play video
     */
    CicadaPlaybackTypeVideo = 1,
    /**
     * @brief 播放音频
     */
    /****
     * @brief play audio
     */
    CicadaPlaybackTypeAudio = 2,
} CicadaPlaybackType;

/**
 @brief CicadaErrorModel为播放错误信息描述
 */
/****
 @brief CicadaErrorModel represents playback error descriptions.
 */
OBJC_EXPORT
@interface CicadaErrorModel : NSObject

/**
 @brief code为播放错误信息code
 */
/****
 @brief code represents a playback error code.
 */
@property (nonatomic, assign) CicadaErrorCode code;

/**
 @brief message为播放错误信息描述
 */
/****
 @brief message represents a playback error message.
 */
@property (nonatomic, copy) NSString *message;

/**
 @brief requestId为播放错误信息requestID
 */
/****
 @brief requestId represents the request ID of a playback error.
 */
@property (nonatomic, copy) NSString *requestId;

/**
 @brief videoId为播放错误发生的videoID
 */
/****
 @brief videoId represents the VID of the video that has a playback error.
 */
@property (nonatomic, copy) NSString *videoId;

@end
/**
 @brief logLevel
 */
typedef enum CicadaLogLevel: NSUInteger {
    CICADA_LOG_LEVEL_NONE    = 0,
    CICADA_LOG_LEVEL_FATAL   = 8,
    CICADA_LOG_LEVEL_ERROR   = 16,
    CICADA_LOG_LEVEL_WARNING = 24,
    CICADA_LOG_LEVEL_INFO    = 32,
    CICADA_LOG_LEVEL_DEBUG   = 48,
    CICADA_LOG_LEVEL_TRACE   = 56,
} CicadaLogLevel;

#endif /* CicadaDef_h */
