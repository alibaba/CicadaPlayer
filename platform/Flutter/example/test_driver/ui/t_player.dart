import 'package:flutter/material.dart';
import 'package:flutter_cicadaplayer/flutter_cicadaplayer.dart';
import 'package:flutter_cicadaplayer/flutter_cicadaplayer_factory.dart';
import 'package:flutter_cicadaplayer_example/config.dart';

void main() => runApp(MyApp());

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    var map = {DataSourceRelated.URL_KEY: DataSourceRelated.DEFAULT_URL};
    return MaterialApp(
      home: TestPlayerPage(playMode: ModeType.URL, dataSourceMap: map),
    );
  }
}

class TestPlayerPage extends StatefulWidget {
  final ModeType playMode;
  final Map<String, dynamic> dataSourceMap;

  TestPlayerPage({Key key, this.playMode, this.dataSourceMap})
      : assert(playMode != null),
        super(key: key);

  @override
  _TestPlayerPageState createState() => _TestPlayerPageState();
}

class _TestPlayerPageState extends State<TestPlayerPage> {
  FlutterCicadaPlayer fCicadaPlayer;
  int bottomIndex;
  List<Widget> mFramePage;
  ModeType _playMode;
  Map<String, dynamic> _dataSourceMap;

  TextEditingController _urlController;

  String _resultStr = "测试中…";

  @override
  void initState() {
    super.initState();
    fCicadaPlayer = FlutterCicadaPlayerFactory().createCicadaPlayer();
    bottomIndex = 0;
    _playMode = widget.playMode;
    _dataSourceMap = widget.dataSourceMap;

    _urlController = TextEditingController();

    _initListener();
  }

  _initListener() {
    fCicadaPlayer.setOnPrepared(() {
      setState(() {
        _resultStr = "准备完成";
      });
    });
    fCicadaPlayer.setOnRenderingStart(() {
      setState(() {
        _resultStr = "首帧渲染";
      });
    });
    fCicadaPlayer.setOnVideoSizeChanged((width, height) {});
    fCicadaPlayer.setOnStateChanged((newState) {
      switch (newState) {
        case FlutterAvpdef.AVPStatus_AVPStatusStarted:
          break;
        case FlutterAvpdef.AVPStatus_AVPStatusPaused:
          break;
        case FlutterAvpdef.stopped:
          setState(() {
            _resultStr = "播放停止";
          });
          break;
        default:
      }
    });

    fCicadaPlayer.setOnCompletion(() {
      setState(() {
        _resultStr = "播放完成";
      });
    });


    // fCicadaPlayer.setOnError((errorCode, errorExtra, errorMsg) {
    //   _showTipsWidget = true;
    //   _showLoading = false;
    //   _tipsContent = "$errorCode \n $errorMsg";
    //   setState(() {});
    // });

  }

  @override
  void dispose() {
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    var x = 0.0;
    var y = 0.0;
    Orientation orientation = MediaQuery.of(context).orientation;
    var width = MediaQuery.of(context).size.width;

    var height;
    if (orientation == Orientation.portrait) {
      height = width * 9.0 / 16.0;
    } else {
      height = MediaQuery.of(context).size.height;
    }
    CicadaPlayerView cicadaPlayerView = CicadaPlayerView(
        onCreated: onViewPlayerCreated,
        x: x,
        y: y,
        width: width,
        height: height);
    return OrientationBuilder(
      builder: (BuildContext context, Orientation orientation) {
        return Scaffold(
          appBar: AppBar(
            title: Text("TestCase"),
          ),
          body: Column(
            children: [
              Stack(
                children: [
                  Container(
                      color: Colors.black,
                      child: cicadaPlayerView,
                      width: width,
                      height: height),
                ],
              ),
              _buildControlBtns(orientation),
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                crossAxisAlignment: CrossAxisAlignment.center,
                children: [
                  Expanded(
                    child: TextField(
                      key: ValueKey("urlField"),
                      controller: _urlController,
                    ),
                  ),
                  TextButton(
                    key: ValueKey("urlBtn"),
                    child: Text("确定"),
                    onPressed: () {
                      this.fCicadaPlayer.setUrl(_urlController.text);
                    },
                  )
                ],
              ),
              Padding(
                padding: const EdgeInsets.all(8.0),
                child: Text(
                  _resultStr,
                  key: ValueKey("resultText"),
                  style: TextStyle(color: Colors.red, fontSize: 17),
                ),
              )
            ],
          ),
        );
      },
    );
  }

  void onViewPlayerCreated() async {
    switch (_playMode) {
      case ModeType.URL:
        // this.fCicadaPlayer.setUrl(_dataSourceMap[DataSourceRelated.URL_KEY]);
        break;
      default:
    }
  }

  /// MARK: 私有方法
  _buildControlBtns(Orientation orientation) {
    return Offstage(
      offstage: orientation == Orientation.landscape,
      child: Padding(
        padding: const EdgeInsets.only(top: 16),
        child: Row(
          mainAxisAlignment: MainAxisAlignment.spaceEvenly,
          crossAxisAlignment: CrossAxisAlignment.center,
          children: [
            InkWell(
                key: ValueKey("prepare"),
                child: Text('准备'),
                onTap: () {
                  fCicadaPlayer.prepare();
                }),
            InkWell(
                key: ValueKey("play"),
                child: Text('播放'),
                onTap: () {
                  fCicadaPlayer.play();
                }),
            InkWell(
              key: ValueKey("stop"),
              child: Text('停止'),
              onTap: () {
                fCicadaPlayer.stop();
              },
            ),
            InkWell(
                key: ValueKey("pause"),
                child: Text('暂停'),
                onTap: () {
                  fCicadaPlayer.pause();
                }),
            InkWell(
                child: Text('截图'),
                onTap: () {
                  // if (Platform.isIOS) {
                  //   fCicadaPlayer.snapshot(
                  //       DateTime.now().millisecondsSinceEpoch.toString() +
                  //           ".png");
                  // } else {
                  //   fCicadaPlayer.snapshot(_snapShotPath +
                  //       "/snapshot_" +
                  //       new DateTime.now().millisecondsSinceEpoch.toString() +
                  //       ".png");
                  // }
                }),
          ],
        ),
      ),
    );
  }
}
