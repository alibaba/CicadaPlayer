import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_cicadaplayer_example/config.dart';
import 'package:flutter_cicadaplayer_example/page/player_page.dart';
import 'package:flutter_cicadaplayer_example/util/common_utils.dart';
import 'package:qrcode_reader/qrcode_reader.dart';

class UrlPage extends StatefulWidget {
  @override
  _UrlPageState createState() => _UrlPageState();
}

class _UrlPageState extends State<UrlPage> {
  TextEditingController urlSourceController =
      new TextEditingController.fromValue(TextEditingValue(
    text: DataSourceRelated.DEFAULT_URL,
  ));
  String _qrcode_result = DataSourceRelated.DEFAULT_URL;

  bool _inPushing = false;

  Future<void> getQrcodeState() async {
    _qrcode_result = await QRCodeReader()
        .setAutoFocusIntervalInMs(200) // default 5000
        .setForceAutoFocus(true) // default false
        .setTorchEnabled(true) // default false
        .setHandlePermissions(true) // default true
        .setExecuteAfterPermissionGranted(true) // default true
        .setFrontCamera(false) // default false
        .scan();

    setState(() {
      urlSourceController.text = _qrcode_result;
    });
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text("URL 播放"),
        centerTitle: true,
        actions: [
          IconButton(
              icon: Icon(Icons.crop_free), onPressed: () => getQrcodeState())
        ],
      ),
      body: Column(
        children: [
          TextField(
            controller: urlSourceController,
            maxLines: 1,
            decoration: InputDecoration(
              labelText: "url",
            ),
            keyboardType: TextInputType.number,
          ),
          RaisedButton(
            child: Text("开始播放"),
            onPressed: () {
              if (_inPushing == true) {
                return;
              }
              _inPushing = true;
              var map = {DataSourceRelated.URL_KEY: urlSourceController.text};
              CommomUtils.pushPage(context,
                  PlayerPage(playMode: ModeType.URL, dataSourceMap: map));
              _inPushing = false;
            },
          )
        ],
      ),
    );
  }
}
