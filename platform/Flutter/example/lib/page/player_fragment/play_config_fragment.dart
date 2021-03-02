import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter/cupertino.dart';
import 'package:flutter_cicadaplayer/flutter_cicadaplayer.dart';
import 'package:fluttertoast/fluttertoast.dart';

class PlayConfigFragment extends StatefulWidget {
  final FlutterCicadaPlayer fCicadaPlayer;
  PlayConfigFragment(this.fCicadaPlayer);

  @override
  _PlayConfigFragmentState createState() => _PlayConfigFragmentState();
}

class _PlayConfigFragmentState extends State<PlayConfigFragment> {
  TextEditingController _mStartBufferDurationController;
  TextEditingController _mHighBufferDurationController;
  TextEditingController _mMaxBufferDurationController;
  TextEditingController _mMaxDelayTimeController;
  TextEditingController _mNetworkTimeoutController;
  TextEditingController _mNetworkRetryCountController;
  TextEditingController _mMaxProbeSizeController;
  TextEditingController _mReferrerController;
  TextEditingController _mHttpProxyController;

  bool mDisableVideo = false;
  bool mDisableAudio = false;
  bool mShowFrameWhenStop = true;
  dynamic _configMap;

  @override
  void initState() {
    super.initState();
    _getPlayerConfig();
  }

  _getPlayerConfig() async {
    _configMap = await widget.fCicadaPlayer.getConfig();
    _mStartBufferDurationController =
        TextEditingController.fromValue(TextEditingValue(
      text: _configMap['mStartBufferDuration'].toString(),
    ));
    _mHighBufferDurationController =
        TextEditingController.fromValue(TextEditingValue(
      text: _configMap['mHighBufferDuration'].toString(),
    ));
    _mMaxBufferDurationController =
        TextEditingController.fromValue(TextEditingValue(
      text: _configMap['mMaxBufferDuration'].toString(),
    ));
    _mMaxDelayTimeController = TextEditingController.fromValue(TextEditingValue(
      text: _configMap['mMaxDelayTime'].toString(),
    ));
    _mNetworkTimeoutController =
        TextEditingController.fromValue(TextEditingValue(
      text: _configMap['mNetworkTimeout'].toString(),
    ));
    _mNetworkRetryCountController =
        TextEditingController.fromValue(TextEditingValue(
      text: _configMap['mNetworkRetryCount'].toString(),
    ));
    _mMaxProbeSizeController = TextEditingController.fromValue(TextEditingValue(
      text: _configMap['mMaxProbeSize'].toString(),
    ));
    _mReferrerController = TextEditingController.fromValue(TextEditingValue(
      text: _configMap['mReferrer'],
    ));
    _mHttpProxyController = TextEditingController.fromValue(TextEditingValue(
      text: _configMap['mHttpProxy'],
    ));
    mShowFrameWhenStop = !_configMap['mClearFrameWhenStop'];
    setState(() {});
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
                controller: _mStartBufferDurationController,
                decoration: InputDecoration(
                  labelText: "起播缓冲区",
                ),
                keyboardType: TextInputType.number,
              ),
              TextField(
                maxLines: 1,
                controller: _mHighBufferDurationController,
                decoration: InputDecoration(
                  labelText: "卡顿恢复",
                ),
                keyboardType: TextInputType.number,
              ),
              TextField(
                maxLines: 1,
                controller: _mMaxBufferDurationController,
                decoration: InputDecoration(
                  labelText: "最大缓冲区",
                ),
                keyboardType: TextInputType.number,
              ),
              TextField(
                maxLines: 1,
                controller: _mMaxDelayTimeController,
                decoration: InputDecoration(
                  labelText: "直播最大延时",
                ),
                keyboardType: TextInputType.number,
              ),
              TextField(
                maxLines: 1,
                controller: _mNetworkTimeoutController,
                decoration: InputDecoration(
                  labelText: "网络超时",
                ),
                keyboardType: TextInputType.number,
              ),
              TextField(
                maxLines: 1,
                controller: _mNetworkRetryCountController,
                decoration: InputDecoration(
                  labelText: "网络重试次数",
                ),
                keyboardType: TextInputType.number,
              ),
              TextField(
                maxLines: 1,
                controller: _mMaxProbeSizeController,
                decoration: InputDecoration(
                  labelText: "probe大小",
                ),
                keyboardType: TextInputType.number,
              ),
              TextField(
                maxLines: 1,
                controller: _mReferrerController,
                decoration: InputDecoration(
                  labelText: "referer",
                ),
              ),
              TextField(
                maxLines: 1,
                controller: _mHttpProxyController,
                decoration: InputDecoration(
                  labelText: "httpProxy",
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
                        value: mDisableVideo,
                        onChanged: (value) {
                          setState(() {
                            mDisableVideo = value;
                          });
                        },
                      ),
                      Text("禁用视频"),
                    ],
                  ),
                  Column(
                    children: [
                      CupertinoSwitch(
                        value: mDisableAudio,
                        onChanged: (value) {
                          setState(() {
                            mDisableAudio = value;
                          });
                        },
                      ),
                      Text("禁用音频"),
                    ],
                  )
                ],
              ),
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceAround,
                children: [
                  // Column(
                  //   children: [
                  //     CupertinoSwitch(
                  //       value: mShowFrameWhenStop,
                  //       onChanged: (value) {
                  //         setState(() {
                  //           mShowFrameWhenStop = value;
                  //         });
                  //       },
                  //     ),
                  //     Text("停止显示最后帧"),
                  //   ],
                  // ),
                  // Column(
                  //   children: [
                  //     CupertinoSwitch(
                  //       value: mEnableSEI,
                  //       onChanged: (value) {
                  //         setState(() {
                  //           mEnableSEI = value;
                  //         });
                  //       },
                  //     ),
                  //     Text("开启SEI"),
                  //   ],
                  // ),
                  InkWell(
                    child: Text(
                      "应用配置",
                      style: TextStyle(color: Colors.blue),
                    ),
                    onTap: () {
                      var configMap = {
                        'mStartBufferDuration':
                            _mStartBufferDurationController.text,
                        'mHighBufferDuratio':
                            _mHighBufferDurationController.text,
                        'mMaxBufferDuration':
                            _mMaxBufferDurationController.text,
                        'mMaxDelayTime': _mMaxDelayTimeController.text,
                        'mNetworkTimeout': _mNetworkTimeoutController.text,
                        'mNetworkRetryCount':
                            _mNetworkRetryCountController.text,
                        'mMaxProbeSize': _mMaxProbeSizeController.text,
                        'mReferrer': _mReferrerController.text,
                        'mHttpProxy': _mHttpProxyController.text,
                        // 'mEnableSEI': mEnableSEI,
                        'mClearFrameWhenStop': !mShowFrameWhenStop,
                        'mDisableVideo': mDisableVideo,
                        'mDisableAudio': mDisableAudio
                      };
                      widget.fCicadaPlayer.setConfig(configMap);
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
