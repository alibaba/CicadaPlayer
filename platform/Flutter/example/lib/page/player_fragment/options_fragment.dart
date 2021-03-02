import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
import 'package:flutter/rendering.dart';
import 'package:flutter_cicadaplayer/flutter_cicadaplayer.dart';
import 'package:flutter_cicadaplayer_example/config.dart';
import 'package:flutter_cicadaplayer_example/widget/cicada_segment.dart';
import 'package:fluttertoast/fluttertoast.dart';

typedef OnEnablePlayBackChanged = Function(bool mEnablePlayBack);

class OptionsFragment extends StatefulWidget {
  final FlutterCicadaPlayer fCicadaPlayer;
  Function playBackChanged;
  _OptionsFragmentState _optionsFragmentState;
  OptionsFragment(this.fCicadaPlayer);

  ///硬解失败切换到软解
  void setOnEnablePlayBackChanged(OnEnablePlayBackChanged enable) {
    this.playBackChanged = enable;
  }

  void switchHardwareDecoder() {
    if (_optionsFragmentState != null) {
      _optionsFragmentState.switchHardwareDecoder();
    }
  }

  @override
  _OptionsFragmentState createState() =>
      _optionsFragmentState = _OptionsFragmentState();
}

class _OptionsFragmentState extends State<OptionsFragment> {
  bool mAutoPlay = false;
  bool mMute = false;
  bool mLoop = false;
  bool mEnableHardwareDecoder = false;
  bool mEnablePlayBack = false;
  int mScaleGroupValue = 0;
  int mMirrorGroupValue = 0;
  int mRotateGroupValue = FlutterAvpdef.AVP_ROTATE_0;
  int mSpeedGroupValueIndex = 0;
  double mSpeedGroupValue = 1;
  double _volume = 100;
  TextEditingController _bgColorController = TextEditingController();

  _loadInitData() async {
    mLoop = await widget.fCicadaPlayer.isLoop();
    mAutoPlay = await widget.fCicadaPlayer.isAutoPlay();
    mMute = await widget.fCicadaPlayer.isMuted();
    mEnableHardwareDecoder = GlobalSettings.mEnableHardwareDecoder;
    mScaleGroupValue = await widget.fCicadaPlayer.getScalingMode();
    mMirrorGroupValue = await widget.fCicadaPlayer.getMirrorMode();
    int rotateMode = await widget.fCicadaPlayer.getRotateMode();
    mRotateGroupValue = (rotateMode / 90).round();
    double speedMode = await widget.fCicadaPlayer.getRate();
    if (speedMode == 0) {
      mSpeedGroupValueIndex = 0;
    } else if (speedMode == 0.5) {
      mSpeedGroupValueIndex = 1;
    } else if (speedMode == 1.5) {
      mSpeedGroupValueIndex = 2;
    } else if (speedMode == 2.0) {
      mSpeedGroupValueIndex = 3;
    }
    double volume = await widget.fCicadaPlayer.getVolume();
    _volume = volume * 100;
    setState(() {});
  }

  ///硬解失败切换到软解
  void switchHardwareDecoder() {
    mEnableHardwareDecoder = false;
    setState(() {});
  }

  @override
  void initState() {
    super.initState();
    _loadInitData();
  }

  @override
  void dispose() {
    super.dispose();
    GlobalSettings.mEnableAccurateSeek = false;
  }

  @override
  Widget build(BuildContext context) {
    return Scrollbar(
      child: SingleChildScrollView(
        child: Container(
          padding: const EdgeInsets.all(5.0),
          child: Column(
            children: [
              _buildSwitch(),
              _buildVolume(),
              _buildScale(),
              _buildMirror(),
              _buildRotate(),
              _buildSpeed(),
              _buildBgColor(),
              _buildPlayBack(),
            ],
          ),
        ),
      ),
    );
  }

  /// switch for : autoplay、mute、loop...
  Row _buildSwitch() {
    return Row(
      mainAxisAlignment: MainAxisAlignment.spaceAround,
      children: [
        Column(
          children: [
            CupertinoSwitch(
              value: mAutoPlay,
              onChanged: (value) {
                setState(() {
                  mAutoPlay = value;
                });
                widget.fCicadaPlayer.setAutoPlay(mAutoPlay);
              },
            ),
            Text("自动播放"),
          ],
        ),
        Column(
          children: [
            CupertinoSwitch(
              value: mMute,
              onChanged: (value) {
                setState(() {
                  mMute = value;
                });
                widget.fCicadaPlayer.setMuted(mMute);
              },
            ),
            Text("静音"),
          ],
        ),
        Column(
          children: [
            CupertinoSwitch(
              value: mLoop,
              onChanged: (value) {
                setState(() {
                  mLoop = !mLoop;
                });
                widget.fCicadaPlayer.setLoop(mLoop);
              },
            ),
            Text("循环"),
          ],
        ),
        Column(
          children: [
            CupertinoSwitch(
              value: mEnableHardwareDecoder,
              onChanged: (value) {},
            ),
            Text("硬解"),
          ],
        ),
        Column(
          children: [
            CupertinoSwitch(
              value: GlobalSettings.mEnableAccurateSeek,
              onChanged: (value) {
                setState(() {
                  GlobalSettings.mEnableAccurateSeek = value;
                });
              },
            ),
            Text("精准seek"),
          ],
        ),
      ],
    );
  }

  /// 音量
  Row _buildVolume() {
    return Row(
      children: [
        SizedBox(
          width: 10.0,
        ),
        Text("音量"),
        Expanded(
          child: Slider(
            value: _volume,
            max: 200,
            onChanged: (value) {
              setState(() {
                _volume = value;
              });
              widget.fCicadaPlayer.setVolume(_volume / 100);
            },
          ),
        ),
      ],
    );
  }

  /// 缩放模式
  Row _buildScale() {
    return Row(
      mainAxisAlignment: MainAxisAlignment.start,
      children: [
        Text('缩放模式'),
        Expanded(
          child: Padding(
            padding: EdgeInsets.symmetric(horizontal: 8),
            child: CicadaSegment(
              titles: ['比例填充', '比例全屏', '拉伸全屏'],
              selIdx: mScaleGroupValue,
              onSelectAtIdx: (value) {
                mScaleGroupValue = value;
                widget.fCicadaPlayer.setScalingMode(mScaleGroupValue);
                setState(() {});
              },
            ),
          ),
        ),
      ],
    );
  }

  /// 镜像模式
  Row _buildMirror() {
    return Row(
      mainAxisAlignment: MainAxisAlignment.start,
      children: [
        Text('镜像模式'),
        Expanded(
          child: Padding(
            padding: EdgeInsets.symmetric(horizontal: 8),
            child: CicadaSegment(
              titles: ['无镜像', '水平镜像', '垂直镜像'],
              selIdx: mMirrorGroupValue,
              onSelectAtIdx: (value) {
                mMirrorGroupValue = value;
                widget.fCicadaPlayer.setMirrorMode(mMirrorGroupValue);
                setState(() {});
              },
            ),
          ),
        ),
      ],
    );
  }

  /// 旋转模式
  Container _buildRotate() {
    double width = MediaQuery.of(context).size.width;
    return Container(
      child: Row(mainAxisAlignment: MainAxisAlignment.start, children: [
        Text('旋转模式'),
        Expanded(
          child: Padding(
            padding: EdgeInsets.symmetric(horizontal: 8),
            child: CicadaSegment(
              titles: ['0°', '90°', '180°', "270°"],
              selIdx: mRotateGroupValue,
              onSelectAtIdx: (value) {
                mRotateGroupValue = value;
                widget.fCicadaPlayer.setRotateMode(mRotateGroupValue * 90);
                setState(() {});
              },
            ),
          ),
        ),
      ]),
    );
  }

  /// 倍速播放
  Row _buildSpeed() {
    return Row(mainAxisAlignment: MainAxisAlignment.spaceAround, children: [
      Text('倍速播放'),
      Expanded(
        child: Padding(
          padding: EdgeInsets.symmetric(horizontal: 8),
          child: CicadaSegment(
            titles: ['正常', '0.5倍速', '1.5倍速', "2.0倍速"],
            selIdx: mSpeedGroupValueIndex,
            onSelectAtIdx: (value) {
              mSpeedGroupValueIndex = value;
              switch (value) {
                case 0:
                  mSpeedGroupValue = 1.0;
                  break;
                case 1:
                  mSpeedGroupValue = 0.5;
                  break;
                case 2:
                  mSpeedGroupValue = 1.5;
                  break;
                case 3:
                  mSpeedGroupValue = 2.0;
                  break;
                default:
              }
              widget.fCicadaPlayer.setRate(mSpeedGroupValue);
              setState(() {});
            },
          ),
        ),
      ),
    ]);
  }

  /// 背景色
  Row _buildBgColor() {
    return Row(
      mainAxisAlignment: MainAxisAlignment.spaceAround,
      children: [
        SizedBox(
          width: 10.0,
        ),
        Text("背景色"),
        SizedBox(
          width: 20.0,
        ),
        Expanded(
          child: TextField(
            maxLines: 1,
            maxLength: 20,
            controller: _bgColorController,
          ),
        ),
        SizedBox(
          width: 30.0,
        ),
        InkWell(
          onTap: () {
            String colorStr = _bgColorController.text;
            if (colorStr.startsWith('#')) {
              colorStr = colorStr.replaceRange(0, 1, '0xff');
            }
            int color = int.tryParse(colorStr);
            if (color != null) {
              widget.fCicadaPlayer.setVideoBackgroundColor(color);
            } else {
              Fluttertoast.showToast(msg: '请输入正确的色值');
            }
          },
          child: Text(
            "确定",
            style: TextStyle(color: Colors.blue),
          ),
        ),
        SizedBox(
          width: 10.0,
        ),
      ],
    );
  }

  /// 后台播放
  Row _buildPlayBack() {
    return Row(
      mainAxisAlignment: MainAxisAlignment.spaceBetween,
      children: [
        Column(
          children: [
            CupertinoSwitch(
              value: mEnablePlayBack,
              onChanged: (value) {
                if (widget.playBackChanged != null) {
                  widget.playBackChanged(value);
                }
                setState(() {
                  mEnablePlayBack = value;
                });
              },
            ),
            Text("后台播放"),
          ],
        ),
        InkWell(
          child: Text(
            "媒体信息",
            style: TextStyle(color: Colors.blue),
          ),
          onTap: () {
            if (widget.fCicadaPlayer != null) {
              widget.fCicadaPlayer.getMediaInfo().then((value) {
                Fluttertoast.showToast(msg: value.toString());
              });
            }
          },
        ),
      ],
    );
  }
}
