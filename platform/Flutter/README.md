# flutter_cicadaplayer

A new flutter plugin project , The project supports Android and iOS base on AliyunPlayerSDK
[Android SDK URL](https://help.aliyun.com/document_detail/94328.html?spm=a2c4g.11186623.6.979.1d5c67b4gEmBvH)
[iOS SDK URL](https://help.aliyun.com/document_detail/94428.html?spm=a2c4g.11186623.6.980.7fc22a88xOI4gc)

## Installation
```yaml
dependencies:
  flutter_cicadaplayer: ^{{latest version}}
```


## Example

```dart
import 'package:flutter_cicadaplayer/flutter_cicadaplayer.dart';
import 'package:flutter_cicadaplayer/flutter_cicadaplayer_factory.dart';


class PlayerPage extends StatefulWidget {
  final ModeType playMode;
  final Map<String, dynamic> dataSourceMap;

  PlayerPage({Key key, this.playMode, this.dataSourceMap})
      : assert(playMode != null),
        super(key: key);

  @override
  _PlayerPageState createState() => _PlayerPageState();
}

class _PlayerPageState extends State<PlayerPage> with WidgetsBindingObserver {
  FlutterAliplayer fAliplayer;
  ModeType _playMode;
  Map<String, dynamic> _dataSourceMap;

  @override
  void initState() {
    super.initState();
    //创建播放器
    fAliplayer = FlutterAliPlayerFactory().createAliPlayer();
    _initListener();
  }

    ///设置监听
  _initListener() {
    fAliplayer.setOnPrepard(() {});
    fAliplayer.setOnRenderingStart(() {});
    fAliplayer.setOnVideoSizeChanged((width, height) {});
    fAliplayer.setOnStateChanged((newState) {});
    fAliplayer.setOnLoadingStatusListener(
        loadingBegin: () {},
        loadingProgress: (percent, netSpeed) {},
        loadingEnd: () {});
    fAliplayer.setOnSeekComplete(() {});
    fAliplayer.setOnInfo((infoCode, extraValue, extraMsg) {});
    fAliplayer.setOnCompletion(() {});
    fAliplayer.setOnTrackReady(() {});
    fAliplayer.setOnSnapShot((path) {});
    fAliplayer.setOnError((errorCode, errorExtra, errorMsg) {});
    fAliplayer.setOnTrackChanged((value) {});
    fAliplayer.setOnThumbnailPreparedListener(
        preparedSuccess: () {}, preparedFail: () {});
  }

  @override
  void dispose() {
    fAliplayer.stop();
    fAliplayer.destroy();
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
    AliPlayerView aliPlayerView = AliPlayerView(
        onCreated: onViewPlayerCreated,
        x: x,
        y: y,
        width: width,
        height: height);
    return OrientationBuilder(
      builder: (BuildContext context, Orientation orientation) {
        return Scaffold(
          body: Column(
            children: [
              Container(
                  color: Colors.black,
                  child: aliPlayerView,
                  width: width,
                  height: height),
            ],
          ),
        );
      },
    );
  }

    ///设置播放源
  void onViewPlayerCreated() async {
    switch (_playMode) {
      case ModeType.URL:
        this.fAliplayer.setUrl(_dataSourceMap[DataSourceRelated.URL_KEY]);
        break;
      case ModeType.STS:
        this.fAliplayer.setVidSts(
            vid: _dataSourceMap[DataSourceRelated.VID_KEY],
            region: _dataSourceMap[DataSourceRelated.REGION_KEY],
            accessKeyId: _dataSourceMap[DataSourceRelated.ACCESSKEYID_KEY],
            accessKeySecret:
                _dataSourceMap[DataSourceRelated.ACCESSKEYSECRET_KEY],
            securityToken: _dataSourceMap[DataSourceRelated.SECURITYTOKEN_KEY],
            definitionList: _dataSourceMap[DataSourceRelated.DEFINITION_LIST],
            previewTime: _dataSourceMap[DataSourceRelated.PREVIEWTIME_KEY]);
        break;
      case ModeType.AUTH:
        this.fAliplayer.setVidAuth(
            vid: _dataSourceMap[DataSourceRelated.VID_KEY],
            region: _dataSourceMap[DataSourceRelated.REGION_KEY],
            playAuth: _dataSourceMap[DataSourceRelated.PLAYAUTH_KEY],
            definitionList: _dataSourceMap[DataSourceRelated.DEFINITION_LIST],
            previewTime: _dataSourceMap[DataSourceRelated.PREVIEWTIME_KEY]);
        break;
      default:
    }
  }
}
```
