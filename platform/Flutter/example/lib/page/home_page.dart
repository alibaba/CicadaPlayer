import 'dart:io';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_cicadaplayer_example/page/setting_page.dart';
import 'package:flutter_cicadaplayer_example/page/url_page.dart';
import 'package:flutter_cicadaplayer_example/util/common_utils.dart';

import 'local_page.dart';

class HomePage extends StatefulWidget {
  @override
  _MyAppState createState() => _MyAppState();
}

class _MyAppState extends State<HomePage> {
  List titleArr = [
    'URL播放',
    '本地文件'
  ];

  @override
  void initState() {
    super.initState();
    SystemChrome.setPreferredOrientations(
        [DeviceOrientation.portraitUp, DeviceOrientation.portraitDown]);
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Plugin for cicadaplayer'),
        actions: [
          IconButton(
            icon: Icon(Icons.settings),
            onPressed: () => CommomUtils.pushPage(context, SettingPage()),
          ),
        ],
      ),
      body: Stack(
        children: [
          ListView.builder(
            padding: EdgeInsets.all(8.0),
            itemExtent: 50.0,
            itemCount: titleArr.length,
            itemBuilder: (BuildContext context, int index) {
              return FlatButton(
                child: Text(titleArr[index]),
                onPressed: () {
                  switch (index) {
                    case 0:
                      CommomUtils.pushPage(context, UrlPage());
                      break;
                    case 1:
                      if (Platform.isAndroid) {
                        CommomUtils.pushPage(context, LocalPage());
                      }
                      break;
                    default:
                  }
                },
              );
            },
          ),
        ],
      ),
    );
  }
}
