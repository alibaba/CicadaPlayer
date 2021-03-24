import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_cicadaplayer/flutter_cicadaplayer.dart';
import 'package:flutter_cicadaplayer/flutter_cicadaplayer_factory.dart';
import 'package:flutter_cicadaplayer_example/mytest/test_resource_model.dart';
import 'package:flutter_cicadaplayer_example/mytest/test_result_model.dart';

import 'package:xml_parser/xml_parser.dart';

class TestPlayerPage extends StatefulWidget {
  @override
  _TestPlayerPageState createState() => _TestPlayerPageState();
}

class _TestPlayerPageState extends State<TestPlayerPage> {

  XmlElement rootNode;

  TestResultModel testResultModel;
  TestResourceModel testResourceModel;

  @override
  void initState() {
    super.initState();

    loadXml();
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
            ],
          ),
        );
      },
    );
  }

  void onViewPlayerCreated() async {}

  void loadXml() async {
    String contents = await rootBundle.loadString("assets/xml/test_case.xml");
    XmlDocument xmlDocument = XmlDocument.fromString(contents);
    rootNode = xmlDocument.getElement('test');

    testResourceModel = TestResourceModel();
    await testResourceModel.load();

    testResultModel = TestResultModel(start: DateTime.now().toString(),children: List<TestResultCaseModel>());
    testFormat();
  }

  // 测试方法
  void testFormat() async {
    List<TestResultItemModel> children = new List<TestResultItemModel>();
    TestResultCaseModel resultCase = TestResultCaseModel(name: 'format',children: children);
    testResultModel.children.add(resultCase);

    FlutterCicadaPlayer fCicadaPlayer = FlutterCicadaPlayerFactory().createCicadaPlayer();
    fCicadaPlayer.setAutoPlay(true);

    final casegroups = rootNode.getElementsWhere(
      name: 'casegroup',
      attributes: [XmlAttribute('casegroup_name', 'format')],
    );
    final resources =
        casegroups.first.getElement('resources').getChildren('id');

    int resIdx = 0;
    
    fCicadaPlayer.setOnCompletion(() {
      TestResultItemModel model =resultCase.children.last;
      model.finish(result:'ok',description: 'ok');
      resIdx++;
      if(resIdx<resources.length){
        fCicadaPlayer.stop();
        XmlElement item = resources[resIdx];
        resultCase.children.add(playItem(fCicadaPlayer, item));
      }else{
        //完成
        close(fCicadaPlayer);
      }
    });

    fCicadaPlayer.setOnError((errorCode, errorExtra, errorMsg) {
      TestResultItemModel model =resultCase.children.last;
      model.finish(result:'failed',description: 'errorCode=$errorCode,errorExtra=$errorExtra,errorMsg=$errorMsg');
      close(fCicadaPlayer);
    });

    XmlElement item = resources[resIdx];
    resultCase.children.add(playItem(fCicadaPlayer, item));
  }

  TestResultItemModel playItem(FlutterCicadaPlayer player, XmlElement item) {
    print('id = ${item.text}');
    TestResourceItemModel model = testResourceModel.findItemById(item.text);
    print('url = ${model.url}');
    player.setUrl(model.url);
    player.prepare();

    TestResultItemModel resultItem = TestResultItemModel(id: item.text,start: DateTime.now().toString());
    return resultItem;
  }

  void close(FlutterCicadaPlayer player) {
    player.stop();
    player.destroy();

    testResultModel.saveToFile();
  }


}
