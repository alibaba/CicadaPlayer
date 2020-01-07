## 一. 播放功能

播放功能的基本流程如下：

创建播放器->设置事件监听->创建播放源->准备播放器->准备成功后开始播放->播放控制->释放播放器。

### 1.创建播放器

可以直接创建CicadaPlayer播放器:

```objectivec
self.player = [[CicadaPlayer alloc] init];
```

### 2.设置播放器监听事件


播放器提供了Delegate回调，比如：onPlayerEvent，onError等事件。使用方法如下：

```
@interface SimplePlayerViewController ()<CicadaDelegate>
@end

- (void)viewDidLoad {
    self.player = [[CicadaPlayer alloc] init];
    self.player.playerView = self.playerView;
    self.player.delegate = self;
    //...
}

/**
 @brief 错误代理回调
 @param player 播放器player指针
 @param errorModel 播放器错误描述，参考CicadaErrorModel
 */
- (void)onError:(CicadaPlayer*)player errorModel:(CicadaErrorModel *)errorModel {
    //提示错误，及stop播放
}

/**
 @brief 播放器事件回调
 @param player 播放器player指针
 @param eventType 播放器事件类型，@see CicadaEventType
 */
-(void)onPlayerEvent:(CicadaPlayer*)player eventType:(CicadaEventType)eventType {
    switch (eventType) {
        case CicadaEventPrepareDone:
            // 准备完成
            break;
        case CicadaEventAutoPlayStart:
            // 自动启播
            break;
        case CicadaEventFirstRenderedStart:
            // 首帧显示
            break;
        case CicadaEventCompletion:
            // 播放完成
            break;
        case CicadaEventLoadingStart:
            // 缓冲开始
            break;
        case CicadaEventLoadingEnd:
            // 缓冲完成
            break;
        case CicadaEventSeekEnd:
            // 跳转完成
            break;
        case CicadaEventLoopingStart:
            // 循环播放开始
            break;
        default:
            break;
    }
}

/**
 @brief 视频当前播放位置回调
 @param player 播放器player指针
 @param position 视频当前播放位置
 */
- (void)onCurrentPositionUpdate:(CicadaPlayer*)player position:(int64_t)position {
    // 更新进度条
}

/**
 @brief 视频缓存位置回调
 @param player 播放器player指针
 @param position 视频当前缓存位置
 */
- (void)onBufferedPositionUpdate:(CicadaPlayer*)player position:(int64_t)position {
    // 更新缓冲进度
}

/**
 @brief 获取track信息回调
 @param player 播放器player指针
 @param info track流信息数组 参考CicadaTrackInfo
 */
- (void)onTrackReady:(CicadaPlayer*)player info:(NSArray<CicadaTrackInfo*>*)info {
    // 获取多码率信息
}

/**
 @brief 字幕显示回调
 @param player 播放器player指针
 @param trackIndex 字幕流索引.
 @param subtitleID  字幕ID.
 @param subtitle 字幕显示的字符串
 */
- (void)onSubtitleShow:(CicadaPlayer*)player trackIndex:(int)trackIndex subtitleID:(long)subtitleID subtitle:(NSString *)subtitle {
    // 获取字幕进行显示
}

/**
 @brief 字幕隐藏回调
 @param player 播放器player指针
 @param trackIndex 字幕流索引.
 @param subtitleID  字幕ID.
 */
- (void)onSubtitleHide:(CicadaPlayer*)player trackIndex:(int)trackIndex subtitleID:(long)subtitleID {
    // 隐藏字幕
}

/**
 @brief 获取截图回调
 @param player 播放器player指针
 @param image 图像
 */
 - (void)onCaptureScreen:(CicadaPlayer*)player image:(CicadaImage*)image {
    // 预览，保存截图
}

/**
 @brief track切换完成回调
 @param player 播放器player指针
 @param info 切换后的信息 参考CicadaTrackInfo
 */
- (void)onTrackChanged:(CicadaPlayer*)player info:(CicadaTrackInfo*)info {
    // 切换码率结果通知
}

//...
```

### 3. 设置播放源

设置网络地址或者本地地址，作为播放源。 


```objectivec
CicadaUrlSource *source = [[CicadaUrlSource alloc] urlWithString:@"网络地址或本机地址"];
[self.player setUrlSource:source];
[self.player prepare];
```

### 4. 设置显示的view

如果源有画面，那么需要设置显示的view到播放器中，用来显示画面。

```objectivec
self.player.playerView = self.avpPlayerView.playerView;//用户显示的view
```

### 5. 播放控制

户自行创建播放器的播放控制按钮，在按钮事件里面实现播放器控制接口。基本控制功能有播放、停止、暂停、拖动（seek），其中Seek功能仅对点播有效，直播使用暂停功能时会使画面停留在当前画面，使用恢复后会开始播放当前画面。使用示例如下：

```objectivec
// 开始播放。
[self.player start];
//暂停播放
[self.player pause];
//停止播放
[self.player stop];
// 跳转到。目前只支持不精准。
[self.player seekToTime:position seekMode:CICADA_SEEKMODE_INACCURATE];
 // 重置
[self.player reset];
//释放。释放后播放器将不可再被使用。
[self.player destroy];
self.player = nil;
```

### 6.切换多码率

播放器SDK支持HLS多码率地址播放。在`prepare`成功之后，通过`getMediaInfo`可以获取到各个码流的信息，即`TrackInfo`。

```objectivec
CicadaMediaInfo *info = [self.player getMediaInfo];
NSArray<CicadaTrackInfo*>* tracks = info.tracks;
```

在播放过程中，可以通过调用播放器的`selectTrack`方法切换播放的码流。

```objectivec
[self.player selectTrack:track.trackIndex];
```

切换的结果会在`onTrackChanged`回调：

```objectivec
- (void)onTrackChanged:(CicadaPlayer*)player info:(CicadaTrackInfo*)info {
    if (info.trackType == CICADA_TRACK_VIDEO) {
        // video changed
    }
    // etc
}
```

### 7.自动播放

播放器SDK支持自动播放视频的设置。在`prepare`之前设置autoPlay。

```objectivec
self.player.autoPlay = YES;
```

设置自动播放之后，prepare成功之后，将会自动播放视频。但是注意：自动播放的时候将不会回调`CicadaEventPrepareDone`回调，而会回调CicadaEventAutoPlayStart回调。

```objectivec
-(void)onPlayerEvent:(CicadaPlayer*)player eventType:(CicadaEventType)eventType {
    switch (eventType) {
        case CicadaEventPrepareDone: {
            break;
        case CicadaEventAutoPlayStart:
            break;
    }
}
```

### 8.循环播放

播放器SDK提供了循环播放视频的功能。设置`loop`开启循环播放, 播放完成后，将会自动从头开始播放视频。

```objectivec
self.player.loop = YES;
```

同时循环开始的回调将在`onPlayerEvent`中使用`CicadaEventLoopingStart`通知。

### 9.画面旋转、填充、镜像操作

播放器SDK提供过了多种设置，可以对画面进行精确的控制。包括设置画面旋转模式，设置画面缩放模式，设置镜像模式。

```objectivec
//设置画面的镜像模式：水平镜像，垂直镜像，无镜像。
self.player.mirrorMode = CICADA_MIRRORMODE_NONE;
//设置画面旋转模式：旋转0度，90度，180度，270度
self.player.rotateMode = CICADA_ROTATE_0;
//设置画面缩放模式：宽高比填充，宽高比适应，拉伸填充
self.player.scalingMode = CICADA_SCALINGMODE_SCALEASPECTFIT;
```

画面的旋转模式包括：

| 值 | 说明 |
| --- | --- |
| CICADA_ROTATE_0 | 顺时针旋转0度 |
| CICADA_ROTATE_90 | 顺时针旋转90度 |
| CICADA_ROTATE_180 | 顺时针旋转180度 |
| CICADA_ROTATE_270 | 顺时针旋转270度 |


画面的缩放模式包括：

| 值 | 说明 |
| --- | --- |
| CICADA_SCALINGMODE_SCALEASPECTFIT | 宽高比适应（将按照视频宽高比等比缩小到view内部，不会有画面变形） |
| CICADA_SCALINGMODE_SCALEASPECTFILL | 宽高比填充（将按照视频宽高比等比放大，充满view，不会有画面变形） |
| CICADA_SCALINGMODE_SCALETOFILL | 拉伸填充（如果视频宽高比例与view比例不一致，会导致画面变形） |


镜像模式包括：

| 值 | 说明 |
| --- | --- |
| CICADA_MIRRORMODE_NONE | 无镜像 |
| CICADA_MIRRORMODE_HORIZONTAL | 水平镜像 |
| CICADA_MIRRORMODE_VERTICAL | 垂直镜像 |


### 10.静音、音量控制

播放器SDK 提供了对视频的音量控制功能。设置muted播放器静音，设置`volume`控制音量大小,范围是0~1。

```objectivec
//设置播放器静音
self.player.muted = YES;
//设置播放器音量,范围0~1.
self.player.volume = 1.0f;
```

### 11.倍数播放

播放器SDK 提供了倍数播放视频的功能，通过设置`rate`方法, 能够以0.5倍~2倍数去播放视频。同时保持变声不变调。

```objectivec
//设置倍速播放:支持0.5~2倍速的播放
self.player.rate = 1.0f;
```

### 12.截图功能

播放器SDK提供了对当前视频截图的功能`snapshot`。截取的是原始的数据，并转为bitmap返回。回调接口为`onCaptureScreen`。 注意：截图是不包含界面的。

```objectivec
//截图回调
- (void)onCaptureScreen:(CicadaPlayer*)player image:(CicadaImage*)image {
    // 处理截图
}

//截取当前播放的画面
[self.player snapShot];
```

### 13. 边播边缓存

播放器SDK提供了边播边缓存的功能，能够让用户重复播放视频时，达到省流量的目的。只需在`prepare`之前给播放器配置`CicadaCacheConfig`即可实现此功能。


```objectivec
CicadaCacheConfig *config = [[CicadaCacheConfig alloc] init];
//开启缓存功能
config.enable = YES;
//能够缓存的单个文件最大时长。超过此长度则不缓存
config.maxDuration = 100;
//缓存目录的位置，需替换成app期望的路径
config.path = @"please use your cache path here";
//缓存目录的最大大小。超过此大小，将会删除最旧的缓存文件
config.maxSizeMB = 200;
//设置缓存配置给到播放器
[self.player setCacheConfig:config];
```

缓存成功之后，以下情况将会利用缓存文件（必须已经设置了`setCacheConfig`）

1. 如果设置了循环播放，那么第二次播放的时候，将会自动播放缓存的文件。
1. 缓存成功后，重新创建播放器，播放同样的资源，也会自动使用缓存文件。

同时，播放器提供了获取缓存文件路径的接口：

```objectivec
-(NSString *) getCacheFilePath:(NSString *)URL;
功能：根据url获取缓存的文件名。必须先调用setCacheConfig才能获取到。
参数：URL URL
返回值：最终缓存的文件绝对路径。
```

边播边缓存也不是所有的视频都会缓存，**有些情况是不会缓存的**。这里详细介绍一下：<br />
1.对于直接播放URL的方式，即`CicadaUrlSource`。如果是HLS(即m3u8)地址，将**不会**缓存。如果是其他支持的格式，则根据缓存配置进行缓存。<br />
2.播放器读取完全部的数据则视为缓存成功。如果在此之前，调用`stop`，或者出错`onError`，则缓存将会失败。<br />
3.cache内的seek的操作不会影响缓存结果。cache外的seek会导致缓存失败。<br />
4.cache的结果回调，会通过`onPlayerEventInfo`回调。

```objectivec
-(void)onPlayerEvent:(CicadaPlayer*)player eventWithString:(CicadaEventWithString)eventWithString description:(NSString *)description {
    if (eventWithString == CICADA_EVENT_PLAYER_CACHE_SUCCESS) {
        //缓存成功事件。
    } else if (eventWithString == CICADA_EVENT_PLAYER_CACHE_ERROR) {
        //缓存失败事件。
    }
}
```

### 14. 硬解开关

播放器SDK 提供了264，265的硬解码能力，同时提供了`enableHardwareDecoder`提供开关。默认开，并且在硬解初始化失败时，自动切换为软解，保证视频的正常播放。

```objectivec
//开启硬解。默认开启
self.player.enableHardwareDecoder = YES;
```

如果从硬解自动切换为软解，将会通过`onPlayerEvent`回调：

```objectivec
-(void)onPlayerEvent:(CicadaPlayer*)player eventWithString:(CicadaEventWithString)eventWithString description:(NSString *)description {
    if (eventWithString == CICADA_EVENT_SWITCH_TO_SOFTWARE_DECODER) {
        //切换到软解
    }
}
```

### 15. 设置Referer

播放器SDK提供了`CicadaConfig` 用来设置请求referer。配合控制台的黑白名单referer，可以控制访问权限。播放器SDK的设置如下：

```objectivec
//先获取配置
CicadaConfig *config = [self.player getConfig];
//设置referer
config.referer = referer;
....//其他设置
//设置配置给播放器
[self.player setConfig:config];
```

### 16. 设置UserAgent

播放器SDK提供了`CicadaConfig` 用来设置请求UA。设置之后，播放器请求的过程中将会带上UA信息。播放器SDK的设置如下：

```objectivec
//先获取配置
CicadaConfig *config = [self.player getConfig];
//设置userAgent
config.userAgent = userAgent;
....//其他设置
//设置配置给播放器
[self.player setConfig:config];
```

### 17. 配置网络重试时间和次数

通过`CicadaConfig`，用户可以设置播放器SDK的网络超时的时间和重试次数。播放器SDK的设置如下：

```objectivec
//先获取配置
CicadaConfig *config = [self.player getConfig];
//设置网络超时时间，单位ms
config.networkTimeout = 5000;
//设置超时重试次数。每次重试间隔为networkTimeout。networkRetryCount=0则表示不重试，重试策略app决定，默认值为2
config.networkRetryCount = 2;
....//其他设置
//设置配置给播放器
[self.player setConfig:config];
```

1. 如果设置了networkRetryCount：如此时发生网络问题，导致出现loading后，那么将会重试networkRetryCount次，每次的间隔时间为networkTimeout。 如果重试多次之后，还是loading的状态，那么就会回调`onError`事件，此时，CicadaErrorModel.code为CICADA_ERROR_LOADING_TIMEOUT。
1. 如果networkRetryCount设置为0，当网络重试超时的时候，播放器就会回调onPlayerEvent，参数eventWithString为CICADA_EVENT_PLAYER_NETWORK_RETRY。 此时，可以调用播放器的`reload`方法进行重新加载网络，或者进行其他的处理。

### 18. 配置缓存和延迟控制

对于播放器来说，缓存的控制非常重要。合理的配置，可以有效的加快起播速度，减少卡顿。播放器SDK通过`CicadaConfig`提供了设置缓存和延迟的控制接口。

```objectivec
//先获取配置
CicadaConfig *config = [self.player getConfig];
//最大延迟。注意：直播有效。当延时比较大时，播放器sdk内部会追帧等，保证播放器的延时在这个范围内。
config.maxDelayTime = 5000;
// 最大缓冲区时长。单位ms。播放器每次最多加载这么长时间的缓冲数据。
config.maxBufferDuration = 50000;
//高缓冲时长。单位ms。当网络不好导致加载数据时，如果加载的缓冲时长到达这个值，结束加载状态。
config.highBufferDuration = 3000;
// 起播缓冲区时长。单位ms。这个时间设置越短，起播越快。也可能会导致播放之后很快就会进入加载状态。
config.startBufferDuration = 500;
//其他设置
//设置配置给播放器
[self.player setConfig:config];
```

> 注意：三个缓冲区时长的大小关系必须为：startBufferDuration<=highBufferDuration<=maxBufferDuration。


### 19. 支持HTTP Header设置

播放器通过CicadaConfig参数，可以给播放器中的请求加上http的header参数。代码如下：

```objectivec
//先获取配置
CicadaConfig *config = [self.player getConfig];
//定义header
NSMutableArray *httpHeaders = [[NSMutableArray alloc] init];
//比如使用httpdns时，需要设置Host。
[httpHeaders addObject:@"Host:xxx.com"];
//设置header
config.httpHeaders = httpHeaders;
....//其他设置
//设置配置给播放器
[self.player setConfig:config];
```
