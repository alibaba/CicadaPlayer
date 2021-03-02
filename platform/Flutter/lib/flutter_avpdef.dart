class FlutterAvpdef {
  /**@brief 不保持比例平铺*/
  /// **@brief Auto stretch to fit.*/
  static const int AVP_SCALINGMODE_SCALETOFILL = 0;
  /**@brief 保持比例，黑边*/
  /// **@brief Keep aspect ratio and add black borders.*/
  static const int AVP_SCALINGMODE_SCALEASPECTFIT = 1;
  /**@brief 保持比例填充，需裁剪*/
  /// **@brief Keep aspect ratio and crop.*/
  static const int AVP_SCALINGMODE_SCALEASPECTFILL = 2;

  /**@brief 旋转模式*/
  /// **@brief Rotate mode*/
  static const int AVP_ROTATE_0 = 0;
  static const int AVP_ROTATE_90 = 90;
  static const int AVP_ROTATE_180 = 180;
  static const int AVP_ROTATE_270 = 270;

  /**@brief 镜像模式*/
  /// **@brief Mirroring mode*/
  static const int AVP_MIRRORMODE_NONE = 0;
  static const int AVP_MIRRORMODE_HORIZONTAL = 1;
  static const int AVP_MIRRORMODE_VERTICAL = 2;

  /// Log 日志级别
  static const int AF_LOG_LEVEL_NONE = 0;
  static const int AF_LOG_LEVEL_FATAL = 8;
  static const int AF_LOG_LEVEL_ERROR = 16;
  static const int AF_LOG_LEVEL_WARNING = 24;
  static const int AF_LOG_LEVEL_INFO = 32;
  static const int AF_LOG_LEVEL_DEBUG = 48;
  static const int AF_LOG_LEVEL_TRACE = 56;

  ///infoCode
  static const int UNKNOWN = -1;
  static const int LOOPINGSTART = 0;
  static const int BUFFEREDPOSITION = 1;
  static const int CURRENTPOSITION = 2;
  static const int AUTOPLAYSTART = 3;
  static const int SWITCHTOSOFTWAREVIDEODECODER = 100;
  static const int AUDIOCODECNOTSUPPORT = 101;
  static const int AUDIODECODERDEVICEERROR = 102;
  static const int VIDEOCODECNOTSUPPORT = 103;
  static const int VIDEODECODERDEVICEERROR = 104;
  static const int VIDEORENDERINITERROR = 105;
  static const int DEMUXERTRACEID = 106;
  static const int NETWORKRETRY = 108;
  static const int CACHESUCCESS = 109;
  static const int CACHEERROR = 110;
  static const int LOWMEMORY = 111;
  static const int NETWORKRETRYSUCCESS = 113;
  static const int SUBTITLESELECTERROR = 114;
  static const int DIRECTCOMPONENTMSG = 116;
  static const int RTSSERVERMAYBEDISCONNECT = 805371905;
  static const int RTSSERVERRECOVER = 805371906;

  ///点播服务器返回的码率清晰度类型
  static const String FD = "FD";
  static const String LD = "LD";
  static const String SD = "SD";
  static const String HD = "HD";
  static const String OD = "OD";
  static const String K2 = "2K";
  static const String K4 = "4K";
  static const String SQ = "SQ";
  static const String HQ = "HQ";
  static const String AUTO = "AUTO";

  ///播放器状态
  static const int unknow = -1;
  static const int idle = 0;
  static const int initalized = 1;
  static const int prepared = 2;
  static const int started = 3;
  static const int paused = 4;
  static const int stopped = 5;
  static const int completion = 6;
  static const int error = 7;

  ///精准seek
  static const int ACCURATE = 1;
  static const int INACCURATE = 16;

  ///下载方式
  static const String DOWNLOADTYPE_STS = "download_sts";
  static const String DOWNLOADTYPE_AUTH = "download_auth";

  ///黑名单
  static const String BLACK_DEVICES_H264 = "HW_Decode_H264";
  static const String BLACK_DEVICES_HEVC = "HW_Decode_HEVC";

  static const int AVPTRACK_TYPE_VIDEO = 0;
  static const int AVPTRACK_TYPE_AUDIO = 1;
  static const int AVPTRACK_TYPE_SUBTITLE = 2;
  static const int AVPTRACK_TYPE_SAAS_VOD = 3;

  //  空转，闲时，静态
  static const int AVPStatus_AVPStatusIdle = 0;
  // /** @brief 初始化完成 */
  static const int AVPStatus_AVPStatusInitialzed = 1;
  // /** @brief 准备完成 */
  static const int AVPStatus_AVPStatusPrepared = 2;
  // /** @brief 正在播放 */
  static const int AVPStatus_AVPStatusStarted = 3;
  // /** @brief 播放暂停 */
  static const int AVPStatus_AVPStatusPaused = 4;
  // /** @brief 播放停止 */
  static const int AVPStatus_AVPStatusStopped = 5;
  // /** @brief 播放完成 */
  static const int AVPStatus_AVPStatusCompletion = 6;
  // /** @brief 播放错误
  static const int AVPStatus_AVPStatusError = 7;
}

class EventChanneldef {
  static const String TYPE_KEY = "method";

  static const String DOWNLOAD_PREPARED = "download_prepared";
  static const String DOWNLOAD_PROGRESS = "download_progress";
  static const String DOWNLOAD_PROCESS = "download_process";
  static const String DOWNLOAD_COMPLETION = "download_completion";
  static const String DOWNLOAD_ERROR = "download_error";
}

class AVPMediaInfo {
  String status;
  String mediaType;
  List<AVPThumbnailInfo> thumbnails;
  List<AVPTrackInfo> tracks;
  String title;
  int duration;
  String transcodeMode;
  String coverURL;

  AVPMediaInfo(
      {this.status,
      this.mediaType,
      this.thumbnails,
      this.tracks,
      this.title,
      this.duration,
      this.transcodeMode,
      this.coverURL});

  AVPMediaInfo.fromJson(Map<dynamic, dynamic> json) {
    status = json['status'];
    mediaType = json['mediaType'];
    if (json['thumbnails'] != null) {
      thumbnails = new List<AVPThumbnailInfo>();
      json['thumbnails'].forEach((v) {
        thumbnails.add(new AVPThumbnailInfo.fromJson(v));
      });
    }
    if (json['tracks'] != null) {
      tracks = new List<AVPTrackInfo>();
      json['tracks'].forEach((v) {
        tracks.add(new AVPTrackInfo.fromJson(v));
      });
    }
    title = json['title'];
    duration = json['duration'];
    transcodeMode = json['transcodeMode'];
    coverURL = json['coverURL'];
  }

  Map<String, dynamic> toJson() {
    final Map<dynamic, dynamic> data = new Map<dynamic, dynamic>();
    data['status'] = this.status;
    data['mediaType'] = this.mediaType;
    if (this.thumbnails != null) {
      data['thumbnails'] = this.thumbnails.map((v) => v.toJson()).toList();
    }
    if (this.tracks != null) {
      data['tracks'] = this.tracks.map((v) => v.toJson()).toList();
    }
    data['title'] = this.title;
    data['duration'] = this.duration;
    data['transcodeMode'] = this.transcodeMode;
    data['coverURL'] = this.coverURL;
    return data;
  }
}

class AVPTrackInfo {
  String vodFormat;
  int videoHeight;
  String subtitleLanguage;
  int videoWidth;
  int trackBitrate;
  int vodFileSize;
  int trackIndex;
  String trackDefinition;
  int audioSampleFormat;
  String audioLanguage;
  String vodPlayUrl;
  int trackType;
  int audioSamplerate;
  int audioChannels;

  AVPTrackInfo(
      {this.vodFormat,
      this.videoHeight,
      this.subtitleLanguage,
      this.videoWidth,
      this.trackBitrate,
      this.vodFileSize,
      this.trackIndex,
      this.trackDefinition,
      this.audioSampleFormat,
      this.audioLanguage,
      this.vodPlayUrl,
      this.trackType,
      this.audioSamplerate,
      this.audioChannels});

  AVPTrackInfo.fromJson(Map<dynamic, dynamic> json) {
    vodFormat = json['vodFormat'];
    videoHeight = json['videoHeight'];
    subtitleLanguage = json['subtitleLanguage'];
    videoWidth = json['videoWidth'];
    trackBitrate = json['trackBitrate'];
    vodFileSize = json['vodFileSize'];
    trackIndex = json['trackIndex'];
    trackDefinition = json['trackDefinition'];
    audioSampleFormat = json['audioSampleFormat'];
    audioLanguage = json['audioLanguage'];
    vodPlayUrl = json['vodPlayUrl'];
    trackType = json['trackType'];
    audioSamplerate = json['audioSamplerate'];
    audioChannels = json['audioChannels'];
  }

  Map<String, dynamic> toJson() {
    final Map<dynamic, dynamic> data = new Map<String, dynamic>();
    data['vodFormat'] = this.vodFormat;
    data['videoHeight'] = this.videoHeight;
    data['subtitleLanguage'] = this.subtitleLanguage;
    data['videoWidth'] = this.videoWidth;
    data['trackBitrate'] = this.trackBitrate;
    data['vodFileSize'] = this.vodFileSize;
    data['trackIndex'] = this.trackIndex;
    data['trackDefinition'] = this.trackDefinition;
    data['audioSampleFormat'] = this.audioSampleFormat;
    data['audioLanguage'] = this.audioLanguage;
    data['vodPlayUrl'] = this.vodPlayUrl;
    data['trackType'] = this.trackType;
    data['audioSamplerate'] = this.audioSamplerate;
    data['audioChannels'] = this.audioChannels;
    return data;
  }
}

class AVPThumbnailInfo {
  String url;

  AVPThumbnailInfo.fromJson(Map<dynamic, dynamic> json) {
    url = json['url'];
  }

  Map<String, dynamic> toJson() {
    final Map<String, dynamic> data = new Map<dynamic, dynamic>();
    data['url'] = this.url;
    return data;
  }
}
