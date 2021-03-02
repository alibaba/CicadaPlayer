import 'dart:io';

import 'package:flutter_cicadaplayer/flutter_avpdef.dart';
import 'package:flutter_cicadaplayer_example/model/definition_model.dart';
import 'package:path_provider/path_provider.dart';

class HttpConstant {
  static const String HTTP_HOST = 'https://alivc-demo.aliyuncs.com/';
  static const String HTTP_METHOD_GET = 'GET';
  static const String HTTP_METHOD_POST = 'POST';

  static const String GET_RECOMMEND_VIDEO_LIST = 'vod/getRecommendVideoList';
}

class GlobalSettings {
  ///软硬解开关
  static bool mEnableHardwareDecoder = true;

  ///播放器日志开关
  static bool mEnableAliPlayerLog = true;

  ///播放器日志级别
  static int mLogLevel = FlutterAvpdef.AF_LOG_LEVEL_INFO;

  ///是否是精准seek
  static bool mEnableAccurateSeek = false;

  ///播放器名称
  static String mPlayerName = "";
}

/// 播放方式
enum ModeType { URL }

enum VideoShowMode { Grid, Srceen }

///播放源相关
class DataSourceRelated {
  static const String DEFAULT_URL =
      "https://alivc-demo-vod.aliyuncs.com/b4da45beb07b4d5b81b54b1ac50fb502/88839cae0df0489584ca3d8217ef8048.m3u8";

  static const String TYPE_KEY = "type";
  static const String REGION_KEY = "region";
  static const String URL_KEY = "url";
  static const String INDEX_KEY = "index";
  static const String PLAYDOMAIN_KEY = "playDomain";
  static const String DEFINITION_LIST = "definitionList";
}
