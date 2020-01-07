## 一. 播放功能

播放功能的基本流程如下：

创建播放器->设置事件监听->创建播放源->准备播放器->准备成功后开始播放->播放控制->释放播放器。

### 1.创建播放器

创建播放器通过`CicadaPlayerFactory`类创建播放器。单个播放功能使用`CicadaPlayer`。创建方法如下:

```java
CicadaPlayer cicadaPlayer;
.....
cicadaPlayer = CicadaPlayerFactory.createCicadaPlayer(getApplicationContext());
```

### 2.设置播放器监听事件

播放器提供了多种监听事件，比如：onPrepared，onCompletion等事件。使用方法如下：

```java
cicadaPlayer.setOnCompletionListener(new CicadaPlayer.OnCompletionListener() {
    @Override
    public void onCompletion() {
        //播放完成事件
    }
});
cicadaPlayer.setOnErrorListener(new CicadaPlayer.OnErrorListener() {
    @Override
    public void onError(ErrorInfo errorInfo) {
        //出错事件
    }
});
cicadaPlayer.setOnPreparedListener(new CicadaPlayer.OnPreparedListener() {
    @Override
    public void onPrepared() {
        //准备成功事件
    }
});
cicadaPlayer.setOnVideoSizeChangedListener(new CicadaPlayer.OnVideoSizeChangedListener() {
    @Override
    public void onVideoSizeChanged(int width, int height) {
        //视频分辨率变化回调
    }

});
cicadaPlayer.setOnRenderingStartListener(new CicadaPlayer.OnRenderingStartListener() {
    @Override
    public void onRenderingStart() {
        //首帧渲染显示事件
    }
});
cicadaPlayer.setOnInfoListener(new CicadaPlayer.OnInfoListener() {
    @Override
    public void onInfo(int type, long extra) {
        //其他信息的事件，type包括了：循环播放开始，缓冲位置，当前播放位置，自动播放开始等
    }
});
cicadaPlayer.setOnLoadingStatusListener(new CicadaPlayer.OnLoadingStatusListener() {
    @Override
    public void onLoadingBegin() {
        //缓冲开始。
    }

    @Override
    public void onLoadingProgress(int percent, float kbps) {
        //缓冲进度
    }

    @Override
    public void onLoadingEnd() {
        //缓冲结束
    }
});
cicadaPlayer.setOnSeekCompleteListener(new CicadaPlayer.OnSeekCompleteListener() {
    @Override
    public void onSeekComplete() {
        //拖动结束
    }
});
cicadaPlayer.setOnSubtitleDisplayListener(new CicadaPlayer.OnSubtitleDisplayListener() {
    @Override
    public void onSubtitleShow(long id, String data) {
        //显示字幕
    }

    @Override
    public void onSubtitleHide(long id) {
        //隐藏字幕
    }
});
cicadaPlayer.setOnTrackChangedListener(new CicadaPlayer.OnTrackChangedListener() {
    @Override
    public void onChangedSuccess(TrackInfo trackInfo) {
        //切换音视频流或者清晰度成功
    }

    @Override
    public void onChangedFail(TrackInfo trackInfo, ErrorInfo errorInfo) {
        //切换音视频流或者清晰度失败
    }
});
cicadaPlayer.setOnStateChangedListener(new CicadaPlayer.OnStateChangedListener() {
    @Override
    public void onStateChanged(int newState) {
        //播放器状态改变事件
    }
});
cicadaPlayer.setOnSnapShotListener(new CicadaPlayer.OnSnapShotListener() {
    @Override
    public void onSnapShot(Bitmap bm, int with, int height) {
        //截图事件
    }
});
```

### 3. 设置播放源

设置网络地址或者本地地址，作为播放源。 

```java
String url = "网络地址或本机地址";
cicadaPlayer.setDataSource(url);
```

### 4. 设置显示的view

如果源有画面，那么需要设置显示的view到播放器中，用来显示画面。支持SurfaceView和TextureView。<br />
以SurfaceView举例：

```java
surfaceView = (SurfaceView) findViewById(R.id.playview);
surfaceView.getHolder().addCallback(new SurfaceHolder.Callback() {
    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        cicadaPlayer.setDisplay(holder);
	}

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
	}

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        cicadaPlayer.setDisplay(null);
	}
});
```

在surfaceChanged方法中需要调用播放器的`redraw`方法，是用来刷新视频画面的。如果view的大小变化了，调用此方法将会更新画面大小，保证视频画面与View的变化一致。

### 5. 播放控制

户自行创建播放器的播放控制按钮，在按钮事件里面实现播放器控制接口。基本控制功能有播放、停止、暂停、拖动（seek），其中Seek功能仅对点播有效，直播使用暂停功能时会使画面停留在当前画面，使用恢复后会开始播放当前画面。使用示例如下：

```java
// 开始播放。
cicadaPlayer.start();
//暂停播放
cicadaPlayer.pause();
//停止播放
cicadaPlayer.stop();
// 跳转到。不精准。
cicadaPlayer.seekTo(long position);
// 重置
cicadaPlayer.reset();
//释放。释放后播放器将不可再被使用。
cicadaPlayer.release();
```

### 6.切换多码率

播放器SDK支持HLS多码率地址播放。在`prepare`成功之后，通过`getMediaInfo`可以获取到各个码流的信息，即`TrackInfo`。

```java
List<TrackInfo> trackInfos  = cicadaPlayer.getMediaInfo().getTrackInfos();
```

在播放过程中，可以通过调用播放器的`selectTrack`方法切换播放的码流。

```java
int index = trackInfo.getIndex();
cicadaPlayer.selectTrack(index);
```

切换的结果会设置`OnTrackChangedListener`监听之后会回调（在调用`selectTrack`之前设置）：

```java
cicadaPlayer.setOnTrackChangedListener(new CicadaPlayer.OnTrackChangedListener() {
    @Override
    public void onChangedSuccess(TrackInfo trackInfo) {
        //切换成功
    }

    @Override
    public void onChangedFail(TrackInfo trackInfo, ErrorInfo errorInfo) {
        //切换失败。失败原因通过errorInfo.getMsg()获取
    }
});
```

### 7.自动播放

播放器SDK支持自动播放视频的设置。在`prepare`之前调用`setAutoPlay`即可设置。

```java
cicadaPlayer.setAutoPlay(true);
```

设置自动播放之后，prepare成功之后，将会自动播放视频。但是注意：自动播放的时候将不会回调`onPrepared`回调，而会回调`onInfo`回调。

```java
cicadaPlayer.setOnInfoListener(new CicadaPlayer.OnInfoListener() {
    @Override
    public void onInfo(InfoBean infoBean) {
        if (infoBean.getCode() == InfoCode.AutoPlayStart){
            //自动播放开始事件。
        }
    }
});
```

### 8.循环播放

播放器SDK提供了循环播放视频的功能。调用`setLoop`开启循环播放, 播放完成后，将会自动从头开始播放视频。

```java
cicadaPlayer.setLoop(true);
```

同时循环开始的回调将会在`onInfo`中通知。

```java
cicadaPlayer.setOnInfoListener(new CicadaPlayer.OnInfoListener() {
    @Override
    public void onInfo(InfoBean infoBean) {
        if (infoBean.getCode() == InfoCode.LoopingStart){
            //循环播放开始事件。
        }
    }
});
```

### 9.画面旋转、填充、镜像操作

播放器SDK提供过了多种设置，可以对画面进行精确的控制。包括设置画面旋转模式，设置画面缩放模式，设置镜像模式。

```java
//设置画面的镜像模式：水平镜像，垂直镜像，无镜像。
cicadaPlayer.setMirrorMode(MirrorMode.MIRROR_MODE_NONE);
//设置画面旋转模式：旋转0度，90度，180度，270度
cicadaPlayer.setRotateMode(RotateMode.ROTATE_0);
//设置画面缩放模式：宽高比填充，宽高比适应，拉伸填充
cicadaPlayer.setScaleMode(ScaleMode.SCALE_ASPECT_FIT);
```

画面的旋转模式包括：

| 值 | 说明 |
| --- | --- |
| ROTATE_0 | 顺时针旋转0度 |
| ROTATE_90 | 顺时针旋转90度 |
| ROTATE_180 | 顺时针旋转180度 |
| ROTATE_270 | 顺时针旋转270度 |


画面的缩放模式包括：

| 值 | 说明 |
| --- | --- |
| SCALE_ASPECT_FIT | 宽高比适应（将按照视频宽高比等比缩小到view内部，不会有画面变形） |
| SCALE_ASPECT_FILL | 宽高比填充（将按照视频宽高比等比放大，充满view，不会有画面变形） |
| SCALE_TO_FILL | 拉伸填充（如果视频宽高比例与view比例不一致，会导致画面变形） |


镜像模式包括：

| 值 | 说明 |
| --- | --- |
| MIRROR_MODE_NONE | 无镜像 |
| MIRROR_MODE_HORIZONTAL | 水平镜像 |
| MIRROR_MODE_VERTICAL | 垂直镜像 |


### 10.静音、音量控制

播放器SDK 提供了对视频的音量控制功能。调用`setMute`设置播放器静音，调用`setVolume`控制音量大小,范围是0~2。

```java
//设置播放器静音
cicadaPlayer.setMute(true);
//设置播放器音量,范围0~2.
cicadaPlayer.setVolume(1f);
```

### 11.倍数播放

播放器SDK 提供了倍数播放视频的功能，通过调用`setSpeed`方法, 能够以0.5倍~2倍数去播放视频。同时保持变声不变调。

```java
//设置倍速播放:支持0.5~2倍速的播放
cicadaPlayer.setSpeed(1.0f);
```

### 12.截图功能

播放器SDK提供了对当前视频截图的功能`snapshot`。截取的是原始的数据，并转为bitmap返回。回调接口为`OnSnapShotListener`。 注意：截图是不包含界面的。

```java
//设置截图回调
cicadaPlayer.setOnSnapShotListener(new OnSnapShotListener(){
    @Override
    public void onSnapShot(Bitmap bm, int with, int height){
        //获取到的bitmap。以及图片的宽高。
    }
});

//截取当前播放的画面
cicadaPlayer.snapshot();
```

### 13. 边播边缓存

播放器SDK提供了边播边缓存的功能，能够让用户重复播放视频时，达到省流量的目的。只需在`prepare`之前给播放器配置`CacheConfig`即可实现此功能。

```java
CacheConfig cacheConfig = new CacheConfig();
//开启缓存功能
cacheConfig.mEnable = true;
 //能够缓存的单个文件最大时长。超过此长度则不缓存
cacheConfig.mMaxDurationS =100;
//缓存目录的位置
cacheConfig.mDir = "缓存的文件目录";
 //缓存目录的最大大小。超过此大小，将会删除最旧的缓存文件
cacheConfig.mMaxSizeMB = 200;
//设置缓存配置给到播放器
cicadaPlayer.setCacheConfig(cacheConfig);
```

缓存成功之后，以下情况将会利用缓存文件（必须已经设置了`setCacheConfig`）

1. 如果设置了循环播放，即`setLoop(true)`，那么第二次播放的时候，将会自动播放缓存的文件。
2. 缓存成功后，重新创建播放器，播放同样的资源，也会自动使用缓存文件。


同时，播放器提供了获取缓存文件路径的接口：

```java
public String getCacheFilePath(String URL)
功能：根据url获取缓存的文件名。必须先调用setCacheConfig才能获取到。
参数：URL URL
返回值：最终缓存的文件绝对路径。
```

边播边缓存也不是所有的视频都会缓存，**有些情况是不会缓存的**。这里详细介绍一下：<br />
1.对于直接播放URL的方式，即`UrlSource`。如果是HLS(即m3u8)地址，将**不会**缓存。如果是其他支持的格式，则根据缓存配置进行缓存。<br />
2.播放器读取完全部的数据则视为缓存成功。如果在此之前，调用`stop`，或者出错`onError`，则缓存将会失败。<br />
3.cache内的seek的操作不会影响缓存结果。cache外的seek会导致缓存失败。<br />
4.cache的结果回调，会通过`onInfo`回调。

```java
cicadaPlayer.setOnInfoListener(new CicadaPlayer.OnInfoListener() {
    @Override
    public void onInfo(InfoBean infoBean) {
        if (infoBean.getCode() == InfoCode.CacheSuccess){
            //缓存成功事件。
        }else if (infoBean.getCode() == InfoCode.CacheError){
            //缓存失败事件。失败原因通过infoBean.getExtraMsg()获取
        }
    }
});
```

### 14. 硬解开关

播放器SDK 提供了264，265的硬解码能力，同时提供了`enableHardwareDecoder`提供开关。默认开，并且在硬解初始化失败时，自动切换为软解，保证视频的正常播放。

```java
//开启硬解。默认开启
cicadaPlayer.enableHardwareDecoder(true);
```

如果从硬解自动切换为软解，将会通过`onInfo`回调：

```java
mApsaraPlayerActivity.setOnInfoListener(new CicadaPlayer.OnInfoListener() {
    @Override
    public void onInfo(InfoBean infoBean) {
        if (infoBean.getCode() == InfoCode.SwitchToSoftwareVideoDecoder) {
            //切换到软解
        }
    }
});
```

### 15. 设置黑名单

播放器SDK提供了硬解的黑名单机制。对于明确不能使用硬解播放的机器，可以直接使用软解，避免了无效的操作。

```java
DeviceInfo deviceInfo = new DeviceInfo();
deviceInfo.model="Lenovo K320t";
CicadaPlayerFactory.addBlackDevice(BlackType.HW_Decode_H264 ,deviceInfo );
```

退出app之后，黑名单自动失效。

### 16. 设置Referer

播放器SDK提供了`PlayerConfig` 用来设置请求referer。配合控制台的黑白名单referer，可以控制访问权限。播放器SDK的设置如下：

```java
//先获取配置
PlayerConfig config = cicadaPlayer.getConfig();
//设置referer
config.mReferrer = referrer;
....//其他设置
  //设置配置给播放器
cicadaPlayer.setConfig(config);
```

### 17. 设置UserAgent

播放器SDK提供了`PlayerConfig` 用来设置请求UA。设置之后，播放器请求的过程中将会带上UA信息。播放器SDK的设置如下：

```java
//先获取配置
PlayerConfig config = cicadaPlayer.getConfig();
//设置UA
config.mUserAgent = "需要设置的UserAgent";
....//其他设置
  //设置配置给播放器
cicadaPlayer.setConfig(config);
```

### 18. 配置网络重试时间和次数

通过`PlayerConfig`，用户可以设置播放器SDK的网络超时的时间和重试次数。播放器SDK的设置如下：

```java
//先获取配置
PlayerConfig config = cicadaPlayer.getConfig();
//设置网络超时时间，单位ms
config.mNetworkTimeout = 5000;
//设置超时重试次数。每次重试间隔为networkTimeout。networkRetryCount=0则表示不重试，重试策略app决定，默认值为2
config.mNetworkRetryCount=2;
....//其他设置
  //设置配置给播放器
cicadaPlayer.setConfig(config);
```

1. 如果设置了NetworkRetryCount：如此时发生网络问题，导致出现loading后，那么将会重试NetworkRetryCount次，每次的间隔时间为mNetworkTimeout。 如果重试多次之后，还是loading的状态，那么就会回调`onError`事件，此时，ErrorInfo.getCode() = ErrorCode.ERROR_LOADING_TIMEOUT。
2. 如果NetworkRetryCount设置为0，当网络重试超时的时候，播放器就会回调`onInfo`事件，事件的InfoBean.getCode() = InfoCode.NetworkRetry。 此时，可以调用播放器的`reload`方法进行重新加载网络，或者进行其他的处理。这块的逻辑可以由App自己来处理。

### 19. 配置缓存和延迟控制

对于播放器来说，缓存的控制非常重要。合理的配置，可以有效的加快起播速度，减少卡顿。播放器SDK通过`PlayerConfig`提供了设置缓存和延迟的控制接口。

```java
//先获取配置
PlayerConfig config = cicadaPlayer.getConfig();
//最大延迟。注意：直播有效。当延时比较大时，播放器sdk内部会追帧等，保证播放器的延时在这个范围内。
config.mMaxDelayTime = 5000;
// 最大缓冲区时长。单位ms。播放器每次最多加载这么长时间的缓冲数据。
config.mMaxBufferDuration = 50000;
//高缓冲时长。单位ms。当网络不好导致加载数据时，如果加载的缓冲时长到达这个值，结束加载状态。
config.mHighBufferDuration = 3000;
// 起播缓冲区时长。单位ms。这个时间设置越短，起播越快。也可能会导致播放之后很快就会进入加载状态。
config.mStartBufferDuration = 500;
....//其他设置
//设置配置给播放器
cicadaPlayer.setConfig(config);
```

> 注意：三个缓冲区时长的大小关系必须为：mStartBufferDuration<=mHighBufferDuration<=mMaxBufferDuration。


### 20. 支持HTTP Header设置

播放器通过`PlayerConfig`参数，可以给播放器中的请求加上http的header参数。代码如下：

```java
//先获取配置
PlayerConfig config = cicadaPlayer.getConfig();
//定义header
String[] headers = new String[1];
headers[0]="Host:xxx.com";//比如需要设置Host到header中。
//设置header
config.setCustomHeaders(headers);
....//其他设置
  //设置配置给播放器
cicadaPlayer.setConfig(config);
```
