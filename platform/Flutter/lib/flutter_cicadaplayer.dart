import 'dart:async';
import 'dart:io';
import 'dart:typed_data';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_cicadaplayer/flutter_avpdef.dart';

import 'flutter_avpdef.dart';

export 'flutter_avpdef.dart';

typedef OnPrepared = void Function();
typedef OnRenderingStart = void Function();
typedef OnVideoSizeChanged = void Function(int width, int height);
typedef OnSnapShot = void Function(String path);

typedef OnSeekComplete = void Function();
typedef OnSeiData = void Function(); //TODO

typedef OnLoadingBegin = void Function();
typedef OnLoadingProgress = void Function(int percent, double netSpeed);
typedef OnLoadingEnd = void Function();

typedef OnStateChanged = void Function(int newState);

typedef OnSubtitleExtAdded = void Function(int trackIndex, String url);
typedef OnSubtitleShow = void Function(
    int trackIndex, int subtitleID, String subtitle);
typedef OnSubtitleHide = void Function(int trackIndex, int subtitleID);
typedef OnTrackReady = void Function();

typedef OnInfo = void Function(int infoCode, int extraValue, String extraMsg);
typedef OnError = void Function(
    int errorCode, String errorExtra, String errorMsg);
typedef OnCompletion = void Function();

typedef OnTrackChanged = void Function(dynamic value);

typedef OnThumbnailPreparedSuccess = void Function();
typedef OnThumbnailPreparedFail = void Function();

typedef OnThumbnailGetSuccess = void Function(
    Uint8List bitmap, Int64List range);
typedef OnThumbnailGetFail = void Function();

class FlutterCicadaPlayer {
  OnLoadingBegin onLoadingBegin;
  OnLoadingProgress onLoadingProgress;
  OnLoadingEnd onLoadingEnd;
  OnPrepared onPrepared;
  OnRenderingStart onRenderingStart;
  OnVideoSizeChanged onVideoSizeChanged;
  OnSeekComplete onSeekComplete;
  OnStateChanged onStateChanged;
  OnInfo onInfo;
  OnCompletion onCompletion;
  OnTrackReady onTrackReady;
  OnError onError;
  OnSnapShot onSnapShot;

  OnTrackChanged onTrackChanged;
  OnThumbnailPreparedSuccess onThumbnailPreparedSuccess;
  OnThumbnailPreparedFail onThumbnailPreparedFail;

  OnThumbnailGetSuccess onThumbnailGetSuccess;
  OnThumbnailGetFail onThumbnailGetFail;

  //外挂字幕
  OnSubtitleExtAdded onSubtitleExtAdded;
  OnSubtitleHide onSubtitleHide;
  OnSubtitleShow onSubtitleShow;

  MethodChannel channel = new MethodChannel('flutter_cicadaplayer');
  EventChannel eventChannel = EventChannel('flutter_cicadaplayer_event');

  FlutterCicadaPlayer.init(int id) {
    eventChannel.receiveBroadcastStream().listen(_onEvent, onError: _onError);
  }

  void setOnPrepared(OnPrepared prepared) {
    this.onPrepared = prepared;
  }

  void setOnRenderingStart(OnRenderingStart renderingStart) {
    this.onRenderingStart = renderingStart;
  }

  void setOnVideoSizeChanged(OnVideoSizeChanged videoSizeChanged) {
    this.onVideoSizeChanged = videoSizeChanged;
  }

  void setOnSnapShot(OnSnapShot snapShot) {
    this.onSnapShot = snapShot;
  }

  void setOnSeekComplete(OnSeekComplete seekComplete) {
    this.onSeekComplete = seekComplete;
  }

  void setOnError(OnError onError) {
    this.onError = onError;
  }

  void setOnLoadingStatusListener(
      {OnLoadingBegin loadingBegin,
      OnLoadingProgress loadingProgress,
      OnLoadingEnd loadingEnd}) {
    this.onLoadingBegin = loadingBegin;
    this.onLoadingProgress = loadingProgress;
    this.onLoadingEnd = loadingEnd;
  }

  void setOnStateChanged(OnStateChanged stateChanged) {
    this.onStateChanged = stateChanged;
  }

  void setOnInfo(OnInfo info) {
    this.onInfo = info;
  }

  void setOnCompletion(OnCompletion completion) {
    this.onCompletion = completion;
  }

  void setOnTrackReady(OnTrackReady onTrackReady) {
    this.onTrackReady = onTrackReady;
  }

  void setOnTrackChanged(OnTrackChanged onTrackChanged) {
    this.onTrackChanged = onTrackChanged;
  }

  void setOnThumbnailPreparedListener(
      {OnThumbnailPreparedSuccess preparedSuccess,
      OnThumbnailPreparedFail preparedFail}) {
    this.onThumbnailPreparedSuccess = preparedSuccess;
    this.onThumbnailPreparedFail = preparedFail;
  }

  void setOnThumbnailGetListener(
      {OnThumbnailGetSuccess onThumbnailGetSuccess,
      OnThumbnailGetFail onThumbnailGetFail}) {
    this.onThumbnailGetSuccess = onThumbnailGetSuccess;
    this.onThumbnailGetSuccess = onThumbnailGetSuccess;
  }

  void setOnSubtitleShow(OnSubtitleShow onSubtitleShow) {
    this.onSubtitleShow = onSubtitleShow;
  }

  void setOnSubtitleHide(OnSubtitleHide onSubtitleHide) {
    this.onSubtitleHide = onSubtitleHide;
  }

  void setOnSubtitleExtAdded(OnSubtitleExtAdded onSubtitleExtAdded) {
    this.onSubtitleExtAdded = onSubtitleExtAdded;
  }

  Future<void> createCicadaPlayer() async {
    return channel.invokeMethod('createCicadaPlayer');
  }

  Future<void> setUrl(String url) async {
    assert(url != null);
    return channel.invokeMethod('setUrl', url);
  }

  Future<void> prepare() async {
    return channel.invokeMethod('prepare');
  }

  Future<void> play() async {
    return channel.invokeMethod('play');
  }

  Future<void> pause() async {
    return channel.invokeMethod('pause');
  }

  Future<dynamic> snapshot(String path) async {
    return channel.invokeMethod('snapshot', path);
  }

  Future<void> stop() async {
    return channel.invokeMethod('stop');
  }

  Future<void> destroy() async {
    return channel.invokeMethod('destroy');
  }

  Future<void> seekTo(int position, int seekMode) async {
    var map = {"position": position, "seekMode": seekMode};
    return channel.invokeMethod("seekTo", map);
  }

  Future<bool> isLoop() async {
    return channel.invokeMethod('isLoop');
  }

  Future<void> setLoop(bool isloop) async {
    return channel.invokeMethod('setLoop', isloop);
  }

  Future<bool> isAutoPlay() async {
    return channel.invokeMethod('isAutoPlay');
  }

  Future<void> setAutoPlay(bool isAutoPlay) async {
    return channel.invokeMethod('setAutoPlay', isAutoPlay);
  }

  Future<bool> isMuted() async {
    return channel.invokeMethod('isMuted');
  }

  Future<void> setMuted(bool isMuted) async {
    return channel.invokeMethod('setMuted', isMuted);
  }

  Future<bool> enableHardwareDecoder() async {
    return channel.invokeMethod('enableHardwareDecoder');
  }

  Future<void> setEnableHardwareDecoder(bool isHardWare) async {
    return channel.invokeMethod('setEnableHardwareDecoder', isHardWare);
  }

  Future<int> getRotateMode() async {
    return channel.invokeMethod('getRotateMode');
  }

  Future<int> getDuration() async {
    return channel.invokeMethod('getDuration');
  }

  Future<void> setRotateMode(int mode) async {
    return channel.invokeMethod('setRotateMode', mode);
  }

  Future<int> getScalingMode() async {
    return channel.invokeMethod('getScalingMode');
  }

  Future<void> setScalingMode(int mode) async {
    return channel.invokeMethod('setScalingMode', mode);
  }

  Future<int> getMirrorMode() async {
    return channel.invokeMethod('getMirrorMode');
  }

  Future<void> setMirrorMode(int mode) async {
    return channel.invokeMethod('setMirrorMode', mode);
  }

  Future<double> getRate() async {
    return channel.invokeMethod('getRate');
  }

  Future<void> setRate(double mode) async {
    return channel.invokeMethod('setRate', mode);
  }

  Future<void> setVideoBackgroundColor(var color) async {
    return channel.invokeMethod('setVideoBackgroundColor', color);
  }

  Future<void> setVolume(double volume) async {
    return channel.invokeMethod('setVolume', volume);
  }

  Future<double> getVolume() async {
    return channel.invokeMethod('getVolume');
  }

  Future<dynamic> getConfig() async {
    return channel.invokeMethod("getConfig");
  }

  Future<void> setConfig(Map map) async {
    return channel.invokeMethod("setConfig", map);
  }

  Future<dynamic> getCacheConfig() async {
    return channel.invokeMethod("getCacheConfig");
  }

  Future<void> setCacheConfig(Map map) async {
    return channel.invokeMethod("setCacheConfig", map);
  }

  ///return deviceInfo
  Future<String> createDeviceInfo() async {
    return channel.invokeMethod("createDeviceInfo");
  }

  ///type : {FlutterAvpdef.BLACK_DEVICES_H264 / FlutterAvpdef.BLACK_DEVICES_HEVC}
  Future<void> addBlackDevice(String type, String model) async {
    var map = {
      'black_type': type,
      'black_device': model,
    };
    return channel.invokeMethod("addBlackDevice", map);
  }

  Future<String> getSDKVersion() async {
    return channel.invokeMethod("getSDKVersion");
  }

  Future<void> enableMix(bool enable) {
    return channel.invokeMethod("enableMix", enable);
  }

  Future<void> enableConsoleLog(bool enable) {
    return channel.invokeMethod("enableConsoleLog", enable);
  }

  Future<void> setLogLevel(int level) async {
    return channel.invokeMethod("setLogLevel", level);
  }

  Future<int> getLogLevel() {
    return channel.invokeMethod("getLogLevel");
  }

  Future<dynamic> getMediaInfo() {
    return channel.invokeMethod("getMediaInfo");
  }

  Future<dynamic> getCurrentTrack(int trackIdx) {
    return channel.invokeMethod("getCurrentTrack", trackIdx);
  }

  Future<dynamic> createThumbnailHelper(String thumbnail) {
    return channel.invokeMethod("createThumbnailHelper", thumbnail);
  }

  Future<dynamic> requestBitmapAtPosition(int position) {
    return channel.invokeMethod("requestBitmapAtPosition", position);
  }

  Future<void> addExtSubtitle(String url) {
    return channel.invokeMethod("addExtSubtitle", url);
  }

  Future<void> selectExtSubtitle(int trackIndex, bool enable) {
    var map = {'trackIndex': trackIndex, 'enable': enable};
    return channel.invokeMethod("selectExtSubtitle", map);
  }

  // accurate 0 为不精确  1 为精确  不填为忽略
  Future<void> selectTrack(int trackIdx, {int accurate = -1}) {
    var map = {
      'trackIdx': trackIdx,
      'accurate': accurate,
    };
    return channel.invokeMethod("selectTrack", map);
  }

  Future<void> setPrivateService(Int8List data) {
    return channel.invokeMethod("setPrivateService", data);
  }

  Future<void> setStreamDelayTime(int trackIdx, int time) {
    var map = {'index': trackIdx, 'time': time};
    return channel.invokeMethod("setStreamDelayTime", map);
  }

  void _onEvent(dynamic event) {
    String method = event[EventChanneldef.TYPE_KEY];
    switch (method) {
      case "onPrepared":
        if (onPrepared != null) {
          onPrepared();
        }
        break;
      case "onRenderingStart":
        if (onRenderingStart != null) {
          onRenderingStart();
        }
        break;
      case "onVideoSizeChanged":
        if (onVideoSizeChanged != null) {
          int width = event['width'];
          int height = event['height'];
          onVideoSizeChanged(width, height);
        }
        break;
      case "onSnapShot":
        if (onSnapShot != null) {
          String snapShotPath = event['snapShotPath'];
          onSnapShot(snapShotPath);
        }
        break;
      case "onChangedSuccess":
        break;
      case "onChangedFail":
        break;
      case "onSeekComplete":
        if (onSeekComplete != null) {
          onSeekComplete();
        }
        break;
      case "onSeiData":
        break;
      case "onLoadingBegin":
        if (onLoadingBegin != null) {
          onLoadingBegin();
        }
        break;
      case "onLoadingProgress":
        int percent = event['percent'];
        double netSpeed = event['netSpeed'];
        if (onLoadingProgress != null) {
          onLoadingProgress(percent, netSpeed);
        }
        break;
      case "onLoadingEnd":
        if (onLoadingEnd != null) {
          print("onLoadingEnd");
          onLoadingEnd();
        }
        break;
      case "onStateChanged":
        if (onStateChanged != null) {
          int newState = event['newState'];
          onStateChanged(newState);
        }
        break;
      case "onInfo":
        if (onInfo != null) {
          int infoCode = event['infoCode'];
          int extraValue = event['extraValue'];
          String extraMsg = event['extraMsg'];
          onInfo(infoCode, extraValue, extraMsg);
        }
        break;
      case "onError":
        if (onError != null) {
          int errorCode = event['errorCode'];
          String errorExtra = event['errorExtra'];
          String errorMsg = event['errorMsg'];
          onError(errorCode, errorExtra, errorMsg);
        }
        break;
      case "onCompletion":
        if (onCompletion != null) {
          onCompletion();
        }
        break;
      case "onTrackReady":
        if (onTrackReady != null) {
          this.onTrackReady();
        }
        break;
      case "onTrackChanged":
        if (onTrackChanged != null) {
          dynamic info = event['info'];
          this.onTrackChanged(info);
        }
        break;
      case "thumbnail_onPrepared_Success":
        if (onThumbnailPreparedSuccess != null) {
          onThumbnailPreparedSuccess();
        }
        break;
      case "thumbnail_onPrepared_Fail":
        if (onThumbnailPreparedFail != null) {
          onThumbnailPreparedFail();
        }
        break;
      case "onThumbnailGetSuccess":
        dynamic bitmap = event['thumbnailbitmap'];
        dynamic range = event['thumbnailRange'];
        if (onThumbnailGetSuccess != null) {
          if (Platform.isIOS) {
            range = Int64List.fromList(range.cast<int>());
          }
          onThumbnailGetSuccess(bitmap, range);
        }
        break;
      case "onThumbnailGetFail":
        if (onThumbnailGetFail != null) {
          onThumbnailGetFail();
        }
        break;
      case "onSubtitleExtAdded":
        if (onSubtitleExtAdded != null) {
          int trackIndex = event['trackIndex'];
          String url = event['url'];
          onSubtitleExtAdded(trackIndex, url);
        }
        break;
      case "onSubtitleShow":
        if (onSubtitleShow != null) {
          int trackIndex = event['trackIndex'];
          int subtitleID = event['subtitleID'];
          String subtitle = event['subtitle'];
          onSubtitleShow(trackIndex, subtitleID, subtitle);
        }
        break;
      case "onSubtitleHide":
        if (onSubtitleHide != null) {
          int trackIndex = event['trackIndex'];
          int subtitleID = event['subtitleID'];
          onSubtitleHide(trackIndex, subtitleID);
        }
        break;
    }
  }

  void _onError(dynamic error) {}
}

typedef void AliPlayerViewCreatedCallback();

class CicadaPlayerView extends StatefulWidget {
  final AliPlayerViewCreatedCallback onCreated;
  final x;
  final y;
  final width;
  final height;

  CicadaPlayerView({
    Key key,
    @required this.onCreated,
    @required this.x,
    @required this.y,
    @required this.width,
    @required this.height,
  });

  @override
  State<StatefulWidget> createState() => _VideoPlayerState();
}

class _VideoPlayerState extends State<CicadaPlayerView> {
  @override
  void initState() {
    super.initState();
  }

  @override
  Widget build(BuildContext context) {
    return nativeView();
  }

  nativeView() {
    if (Platform.isAndroid) {
      return AndroidView(
        viewType: 'flutter_cicadaplayer_render_view',
        onPlatformViewCreated: _onPlatformViewCreated,
        creationParams: <String, dynamic>{
          "x": widget.x,
          "y": widget.y,
          "width": widget.width,
          "height": widget.height,
        },
        creationParamsCodec: const StandardMessageCodec(),
      );
    } else {
      return UiKitView(
        viewType: 'plugins.flutter_cicadaplayer',
        onPlatformViewCreated: _onPlatformViewCreated,
        creationParams: <String, dynamic>{
          "x": widget.x,
          "y": widget.y,
          "width": widget.width,
          "height": widget.height,
        },
        creationParamsCodec: const StandardMessageCodec(),
      );
    }
  }

  Future<void> _onPlatformViewCreated(id) async {
    if (widget.onCreated != null) {
      widget.onCreated();
    }
  }
}
