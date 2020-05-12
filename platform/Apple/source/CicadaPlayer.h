//
//  CicadaPlayer.h
//  CicadaPlayer
//
//  Created by shiping.csp on 2018/11/16.
//  Copyright © 2018 com.alibaba.AliyunPlayer. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "CicadaDelegate.h"
#import "CicadaRenderDelegate.h"
#import "CicadaSource.h"
#import "CicadaDef.h"
#import "CicadaMediaInfo.h"
#import "CicadaConfig.h"
#import "CicadaCacheConfig.h"
#import "CicadaAudioSessionDelegate.h"

OBJC_EXPORT
@interface CicadaPlayer : NSObject

/**
 @brief 初始化播放器
 */
/****
 @brief Initialize the player.
 */
- (instancetype)init;

/**
 @brief 初始化播放器
 @param traceID 用于跟踪debug信息
 */
/****
 @brief Initialize the player.
 @param traceID A trace ID for debugging.
 */
- (instancetype)init:(NSString*)traceID;

/**
 @brief 使用url方式来播放视频
 @param source CicadaUrlSource的输入类型
 @see CicadaUrlSource
 */
/****
 @brief Play by URL.
 @param source CicadaUrlSource type.
 @see CicadaUrlSource
 */
- (void)setUrlSource:(CicadaUrlSource*)source;

/**
 @brief 播放准备，异步
 */
/****
 @brief Prepare for playback. Asynchronous callback.
 */
-(void)prepare;

/**
 @brief 开始播放
 */
/****
 @brief Start playing.
 */
-(void)start;

/**
 @brief 暂停播放
 */
/****
 @brief Pause.
 */
-(void)pause;

/**
 @brief 刷新view，例如view size变化时。
 */
/****
 @brief Refresh the view in certain situations, for example, when the size of the view changes.
 */
-(void)redraw;

/**
 @brief 重置播放
 */
/****
 @brief Reset.
 */
-(void)reset;

/**
 @brief 停止播放
 */
/****
 @brief Stop.
 */
-(void)stop;

/**
 @brief 销毁播放器
 */
/****
 @brief Delete the player.
 */
-(void)destroy;

/**
 @brief 跳转到指定的播放位置
 @param time 新的播放位置
 @param seekMode seek模式
 @see CicadaSeekMode
 */
/****
 @brief Seek to a specified position.
 @param time The specified position that the player will seek to.
 @param seekMode Seek mode.
 @see CicadaSeekMode
 */
-(void)seekToTime:(int64_t)time seekMode:(CicadaSeekMode)seekMode;

/**
 * 设置精准seek的最大间隔。
 * @param delta 间隔时间，单位毫秒
 */
/****
* set the maximum interval of precision seek.
* @param delta interval in milliseconds
*/
-(void)setMaxAccurateSeekDelta:(int)delta;

/**
 @brief 截图 CicadaImage: mac平台返回NSImage，iOS平台返回UIImage
 */
/****
 @brief Snapshot. CicadaImage: For a Mac platform, NSImage is returned. For an iOS platform, UIImage is returned.
 */
-(void) snapShot;

/**
 @brief 根据trackIndex，切换清晰度
 @param trackIndex 选择清晰度的index，SELECT_CICADA_TRACK_VIDEO_AUTO代表自适应码率
 */
/****
 @brief Switch definition according to the specified track index.
 @param trackIndex Specify a definition index. SELECT_CICADA_TRACK_VIDEO_AUTO: auto bitrate adjustment.
 */
-(void)selectTrack:(int)trackIndex;

/**
 @brief 获取媒体信息，包括track信息
 */
/****
 @brief Query media information, including track information.
 */
-(CicadaMediaInfo*) getMediaInfo;

/**
 @brief 获取当前播放track
 @param type track类型
 @see CicadaTrackType
 */
/****
 @brief Query the track that is being played.
 @param type Track type.
 @see CicadaTrackType
 */
-(CicadaTrackInfo*) getCurrentTrack:(CicadaTrackType)type;

/**
 @brief 设置缩略图URL
 @param URL 缩略图URL
 */
/****
 @brief Specify a thumbnail image URL.
 @param URL The specified thumbnail image URL.
 */
-(void) setThumbnailUrl:(NSString *)URL;

/**
 @brief 获取指定位置的缩略图
 @param positionMs 代表在哪个指定位置的缩略图
 */
/****
 @brief Retrieve the thumbnail image at the specified position.
 @param positionMs The specified position.
 */
-(void)getThumbnail:(int64_t)positionMs;

/**
 @brief 播放器设置
 @param config CicadaConfig类型
 @see CicadaConfig
 */
/****
 @brief Modify player configuration.
 @param config CicadaConfig type.
 @see CicadaConfig
 */
-(void) setConfig:(CicadaConfig*)config;

/**
 @brief 获取播放器设置
 @see CicadaConfig
 */
/****
 @brief Query player configuration.
 @see CicadaConfig
 */
-(CicadaConfig*) getConfig;

/**
 @brief 设置缓存配置
 @param CicadaCacheConfig 缓存配置。{@link CicadaCacheConfig}。
 @return 设置成功返回YES
 */
/****
 @brief Modify cache configuration.
 @param CicadaCacheConfig Cache configuration. {@link CicadaCacheConfig}.
 @return If the cache configuration is modified, YES is returned.
 */
-(BOOL) setCacheConfig:(CicadaCacheConfig *)cacheConfig;

/**
 @brief 根据url获取缓存的文件名。如果有自定义的规则，请实现delegate {@link onGetCacheNameByURL}。
 @brief 如果没有实现delegate，将会采用默认逻辑生成文件名。
 @param URL URL
 * @return 最终缓存的文件名（不包括后缀。缓存文件的后缀名统一为.alv）。
 */
/****
 @brief Query the name of the cached file with the specified URL. If a custom rule is set, delegate {@link onGetCacheNameByURL}.
 @brief If the delegation failed, the default logic is used to generate the file name.
 @param URL The URL of the cached file.
 * @return The name of the cached file. The file extension is not included. All files use the .alv extension.
 */
-(NSString *) getCacheFilePath:(NSString *)URL;

/**
 @brief 添加外挂字幕。
 @param URL 字幕地址
 */
/****
 @brief Add external subtitles
 @param URL subtitle address
 */
-(void) addExtSubtitle:(NSString *)URL;

/**
 @brief 选择外挂字幕
 @param trackIndex 字幕索引
 @param enable true：选择，false：关闭
 */
/****
 @brief Select external subtitles
 @param trackIndex caption index
 @param enable true: select, false: close
 */
-(void) selectExtSubtitle:(int)trackIndex enable:(BOOL)enable;

/**
 @brief 重新加载。比如网络超时时，可以重新加载。
 */
/****
 @brief Reload resources. You can reload resources when the network connection times out.
 */
-(void) reload;

/**
 @brief 根据key获取相应的信息。
 @param key 关键字枚举值
 * @return 相应信息（找不到相应信息返回空字符串）。
 */
/****
 @brief Get information by key.
 @param key The enum of key
 * @return corresponding information, return "" if doesn't exist.
 */
-(NSString *) getPropertyString:(CicadaPropertyKey)key;

/**
 @brief 设置多码率时默认播放的码率。将会选择与之最接近的一路流播放。
 @param bandWidth 播放的码率。
 */
/****
 @brief Set the default playback bitrate for multi-bit rate. The nearest stream will be selected.
 @param bandWidth bit rate .
 */
-(void) setDefaultBandWidth:(int)bandWidth;

/**
 @brief 设置所需的播放内容。
 @param CicadaPlaybackType 指定播放音频或视频。
 */
/****
 @brief Set the playback track.
 @param CicadaPlaybackType The specified track to playback.
 */
-(void) setPlaybackType:(CicadaPlaybackType)type;

/**
 @brief 获取当前播放的pts。
 @return 当前播放的pts。
 */
/****
 @brief Get playing pts.
 @return Playing pts.
 */
-(int64_t) getPlayingPts;


/**
 @brief 设置参考时间钟。
 @param referClock 参考时间钟。
 */
/****
 @brief Set the refer clock
 @param referClock The refer clock
 */
-(void) SetClockRefer:(int64_t (^)(void))referClock;

/**
 * @brief 获取播放器的参数
 *
 * @param key 参数值
 * @return 相关信息
 */
/****
 * @brief Get player information
 *
 * @param key The key
 * @return The information
 */
-(NSString *) getOption:(CicadaOption)key;

/**
 @brief 获取SDK版本号信息
 */
/****
 @brief Query the SDK version.
 */
+ (NSString*) getSDKVersion;

/**
 @brief 初始化播放器组件。这些组件是可裁剪的。App可删除相应动态库，去掉初始化组件代码，实现裁剪。
 */
/****
 @brief Initialize player components. These components are optional. You can remove these components by deleting the dynamic libraries and code of the components on your app.
 */
+ (void) initPlayerComponent:(NSString *)functionName function:(void*)function;

/**
 @brief 设置是否静音，支持KVO
 */
/****
 @brief Mute or unmute the player. KVO is supported.
 */
@property(nonatomic, getter=isMuted) BOOL muted;

/**
 @brief 播放速率，0.5-2.0之间，1为正常播放，支持KVO
 */
/****
 @brief Set the playback speed. Valid values: 0.5 to 2.0. Value 1 indicates normal speed. KVO is supported.
 */
@property(nonatomic) float rate;

/**
 @brief 是否开启硬件解码，支持KVO
 */
/****
 @brief Enable or disable hardware decoding. KVO is supported.
 */
@property(nonatomic) BOOL enableHardwareDecoder;

/**
 @brief 设置是否循环播放，支持KVO
 */
/****
 @brief Enable or disable loop playback. KVO is supported.
 */
@property(nonatomic, getter=isLoop) BOOL loop;

/**
 @brief 设置是否自动播放，支持KVO
 */
/****
 @brief Enable or disable autoplay. KVO is supported.
 */
@property(nonatomic, getter=isAutoPlay) BOOL autoPlay;

/**
 @brief 是否打开log输出
 @see 使用setLogCallbackInfo
 */
/****
 @brief Enable or disable logging.
 @see Use setLogCallbackInfo.
 */
@property(nonatomic) BOOL enableLog;

/**
 @brief 渲染镜像模式，支持KVO
 @see CicadaMirrorMode
 */
/****
 @brief Set a mirroring mode. KVO is supported.
 @see CicadaMirrorMode
 */
@property(nonatomic) CicadaMirrorMode mirrorMode;

/**
 @brief 渲染旋转模式，支持KVO
 @see CicadaRotateMode
 */
/****
 @brief Set a rotate mode. KVO is supported.
 @see CicadaRotateMode
 */
@property(nonatomic) CicadaRotateMode rotateMode;

/**
 @brief 渲染填充模式，支持KVO
 @see CicadaScalingMode
 */
/****
 @brief Set a zoom mode. KVO is supported.
 @see CicadaScalingMode
 */
@property(nonatomic) CicadaScalingMode scalingMode;


/**
 @brief 设置播放器的视图playerView
 * CicadaView: mac下为NSOpenGLView，iOS下为UIView
 */
/****
 @brief Set a player view (playerView).
 * CicadaView: Mac platform: NSOpenGLView. iOS platform: UIView.
 */
@property(nonatomic, strong) CicadaView* playerView;

/**
 @brief 获取视频的宽度，支持KVO
 */
/****
 @brief Query the width of the video. KVO is supported.
 */
@property (nonatomic) int width;

/**
 @brief 获取视频的高度，支持KVO
 */
/****
 @brief Query the height of the video. KVO is supported.
 */
@property (nonatomic) int height;

/**
 @brief 获取视频的旋转角度，从metadata中获取出来，支持KVO
 */
/****
 @brief Query the rotate angle of the video, which is retrieved from the video metadata. KVO is supported.
 */
@property (nonatomic, readonly) int rotation;

/**
 @brief 获取/设置播放器的音量（非系统音量），支持KVO，范围0.0~2.0，当音量大于1.0时，可能出现噪音，不推荐使用。
 */
/****
 @brief Query or set the volume of the player(Not system volume). KVO is supported. The range is 0.0~2.0，it maybe lead to noise if set volume more then 1.0, not recommended.
 */
@property (nonatomic, assign) float volume;

/**
 @brief 获取视频的长度，支持KVO
 */
/****
 @brief Query the length of the video. KVO is supported.
 */
@property (nonatomic) int64_t duration;

/**
 @brief 获取当前播放位置，支持KVO
 */
/****
 @brief Query the current playback position. KVO is supported.
 */
@property (nonatomic) int64_t currentPosition;

/**
 @brief 获取已经缓存的位置，支持KVO
 */
/****
 @brief Query the buffered position. KVO is supported.
 */
@property (nonatomic) int64_t bufferedPosition;

/**
 @brief 设置代理，参考CicadaDelegate
 @see CicadaDelegate
 */
/****
 @brief Set a proxy. See CicadaDelegate.
 @see CicadaDelegate
 */
@property (nonatomic, weak) id<CicadaDelegate> delegate;

/**
 * 设置渲染回调。
 */
@property(nonatomic, weak) id <CicadaRenderDelegate> renderDelegate;

- (void)setInnerDelegate:(id<CicadaDelegate>) delegate;

/**
 @brief 设置AudioSession的Delegate
 @param delegate Delegate对象
 */
/****
 @brief 设置AudioSession的Delegate
 @param delegate Delegate对象
 */
+ (void)setAudioSessionDelegate:(id<CicadaAudioSessionDelegate>)delegate;

/**
 @brief 设置日志打印回调block，异步
 @param logLevel log输出级别
 @param callbackBlock log回调block，可以为nil
 */
/****
 @brief Set a log output callback block. Asynchronous.
 @param logLevel The level of the log.
 @param callbackBlock Log output callback block, which can be nil.
 */
+(void) setLogCallbackInfo:(CicadaLogLevel)logLevel callbackBlock:(void (^)(CicadaLogLevel logLevel,NSString* strLog))block;

@end

