import 'dart:io';

import 'package:flutter/material.dart';
import 'package:flutter/cupertino.dart';
import 'package:flutter_cicadaplayer/flutter_cicadaplayer.dart';
import 'package:fluttertoast/fluttertoast.dart';
import 'package:path_provider/path_provider.dart';

class CacheConfigFragment extends StatefulWidget {
  final FlutterCicadaPlayer fCicadaPlayer;
  CacheConfigFragment(this.fCicadaPlayer);

  @override
  _CacheConfigFragmentState createState() => _CacheConfigFragmentState();
}

class _CacheConfigFragmentState extends State<CacheConfigFragment> {
  TextEditingController _mMaxSizeMBController =
      TextEditingController.fromValue(TextEditingValue(text: "500"));
  TextEditingController _mMaxDurationSController =
      TextEditingController.fromValue(TextEditingValue(text: "100"));
  TextEditingController _mDirController;
  String _cacheSavePath;
  bool mEnableCacheConfig = false;

  @override
  void initState() {
    super.initState();
    if (Platform.isAndroid) {
      getExternalStorageDirectories().then((value) {
        if (value.length > 0) {
          _cacheSavePath = value[0].path + "/cache/";
          return Directory(_cacheSavePath);
        }
      }).then((value) {
        return value.exists();
      }).then((value) {
        if (!value) {
          Directory directory = Directory(_cacheSavePath);
          directory.create();
        }
        return _cacheSavePath;
      }).then((value) {
        _mDirController = TextEditingController.fromValue(TextEditingValue(
          text: value,
        ));
      });
    } else if (Platform.isIOS) {
      _mDirController = TextEditingController.fromValue(TextEditingValue(
        text: 'cache',
      ));
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scrollbar(
      child: SingleChildScrollView(
        child: Container(
          margin:
              EdgeInsets.only(top: 10.0, left: 20.0, bottom: 10.0, right: 20.0),
          child: Column(
            children: [
              TextField(
                maxLines: 1,
                controller: _mMaxDurationSController,
                enabled: mEnableCacheConfig,
                decoration: InputDecoration(
                  labelText: "最大时长(s)",
                ),
                keyboardType: TextInputType.number,
              ),
              TextField(
                maxLines: 1,
                controller: _mMaxSizeMBController,
                enabled: mEnableCacheConfig,
                decoration: InputDecoration(
                  labelText: "最大Size(MB)",
                ),
                keyboardType: TextInputType.number,
              ),
              TextField(
                maxLines: 1,
                controller: _mDirController,
                enabled: mEnableCacheConfig,
                decoration: InputDecoration(
                  labelText: "保存路径",
                ),
              ),
              SizedBox(
                height: 30.0,
              ),
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceAround,
                children: [
                  Column(
                    children: [
                      CupertinoSwitch(
                        value: mEnableCacheConfig,
                        onChanged: (value) {
                          setState(() {
                            mEnableCacheConfig = value;
                          });
                        },
                      ),
                      Text("是否开启缓存配置"),
                    ],
                  ),
                  InkWell(
                    child: Text(
                      "应用配置",
                      style: TextStyle(color: Colors.blue),
                    ),
                    onTap: () {
                      var map = {
                        "mMaxSizeMB": _mMaxSizeMBController.text,
                        "mMaxDurationS": _mMaxDurationSController.text,
                        "mDir": _mDirController.text,
                        "mEnable": mEnableCacheConfig,
                      };
                      widget.fCicadaPlayer.setCacheConfig(map);
                      Fluttertoast.showToast(msg: "应用配置成功");
                    },
                  ),
                ],
              ),
            ],
          ),
        ),
      ),
    );
  }
}
