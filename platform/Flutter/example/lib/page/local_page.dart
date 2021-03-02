import 'dart:io';

import 'package:flutter/material.dart';
import 'package:flutter_cicadaplayer_example/config.dart';
import 'package:flutter_cicadaplayer_example/page/player_page.dart';
import 'package:flutter_cicadaplayer_example/util/common_utils.dart';
import 'package:path_provider/path_provider.dart';

class LocalPage extends StatefulWidget {
  @override
  _LocalPageState createState() => _LocalPageState();
}

class _LocalPageState extends State<LocalPage> {
  String _cacheSavePath;
  List<String> _localFileList = List();

  @override
  void initState() {
    super.initState();
    getExternalStorageDirectories().then((value) {
      if (value.length > 0) {
        _cacheSavePath = value[0].path + "/cache/";
        return Directory(_cacheSavePath);
      }
    }).then((value) {
      List<FileSystemEntity> files = value.listSync();
      for (var f in files) {
        var bool = FileSystemEntity.isFileSync(f.path);
        if (bool) {
          _localFileList.add(f.path);
        }
      }
      setState(() {});
    });
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text("本地缓存文件"),
      ),
      body: ListView.separated(
          separatorBuilder: (BuildContext context, int index) =>
              Divider(height: 3.0, color: Colors.black),
          itemCount: _localFileList.length,
          itemBuilder: (BuildContext context, int index) {
            return Column(
              children: [
                InkWell(
                  child: Text("${_localFileList[index]}"),
                  onTap: () {
                    Map<String, String> dataSourcecMap = {
                      DataSourceRelated.URL_KEY: _localFileList[index]
                    };
                    CommomUtils.pushPage(
                        context,
                        PlayerPage(
                            playMode: ModeType.URL,
                            dataSourceMap: dataSourcecMap));
                  },
                ),
              ],
            );
          }),
    );
  }
}
