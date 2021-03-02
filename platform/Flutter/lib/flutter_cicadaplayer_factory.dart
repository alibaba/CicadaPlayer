import 'dart:io';
import 'package:flutter/services.dart';

import 'flutter_cicadaplayer.dart';

class FlutterCicadaPlayerFactory {
  MethodChannel _methodChannel =
      MethodChannel("plugins.flutter_cicadaplayer_factory");

  FlutterCicadaPlayer createCicadaPlayer() {
    if (Platform.isAndroid) {
      _methodChannel.invokeMethod("createCicadaPlayer");
    }
    FlutterCicadaPlayer flutterCicadaPlayer = FlutterCicadaPlayer.init(0);
    return flutterCicadaPlayer;
  }
}
