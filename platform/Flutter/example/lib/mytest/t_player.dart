import 'package:flutter/material.dart';
import 'package:flutter_cicadaplayer/flutter_cicadaplayer.dart';
import 'package:flutter_cicadaplayer_example/mytest/command/command_engine.dart';
import 'package:flutter_cicadaplayer_example/mytest/model/test_case_model.dart';
import 'package:flutter_cicadaplayer_example/mytest/model/test_resource_model.dart';
import 'package:flutter_cicadaplayer_example/mytest/model/test_result_model.dart';

class TestPlayerPage extends StatefulWidget {
  @override
  _TestPlayerPageState createState() => _TestPlayerPageState();
}

class _TestPlayerPageState extends State<TestPlayerPage> {
  TestResultModel testResultModel;
  TestResourceModel testResourceModel;
  TestCaseModel testCaseModel;

  CommandEngine engine;

  @override
  void initState() {
    super.initState();

    loadXml();
  }

  @override
  void dispose() {
    super.dispose();
    engine.clear();
  }

  @override
  Widget build(BuildContext context) {
    var colorList = [Colors.grey,Colors.orange,Colors.green,Colors.red];

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
    return Scaffold(
      appBar: AppBar(
        title: Text("TestCase"),
      ),
      body: Column(
        children: [
          Container(
              color: Colors.black,
              child: cicadaPlayerView,
              width: width,
              height: height),
          Expanded(
            child: Container(
              child: Scrollbar(
                child: SingleChildScrollView(
                  child: ExpansionPanelList(
                    expandedHeaderPadding: EdgeInsets.symmetric(vertical: 1),
                    expansionCallback: (index, bool) {
                      TestCaseGroup group = testCaseModel.contentMap.values
                          .toList()
                          .elementAt(index);
                      setState(() {
                        group.isExpanded = !group.isExpanded;
                      });
                    },
                    children: testCaseModel.contentMap.values
                        .map((item) => ExpansionPanel(
                              isExpanded: item.isExpanded,
                              canTapOnHeader: true,
                              headerBuilder: (context, isExpanded) {
                                return Text('${item.name}',style: TextStyle(fontWeight: FontWeight.bold),);
                              },
                              body: Column(
                                  children: item.children
                                      .map((e) => Padding(
                                        padding: const EdgeInsets.all(16.0),
                                        child: Row(
                                              mainAxisAlignment: MainAxisAlignment.spaceBetween,
                                              children: [
                                                Expanded(child: Text(e.name)),
                                                Text(e.getStatusStr(),
                                                style: TextStyle(color:colorList[e.status]),)
                                              ],
                                            ),
                                      ))
                                      .toList()),
                            ))
                        .toList(),
                  ),
                ),
              ),
            ),
          )
        ],
      ),
    );
  }

  void onViewPlayerCreated() async {}

  void loadXml() async {
    testCaseModel = TestCaseModel();
    await testCaseModel.load();

    testResourceModel = TestResourceModel();
    await testResourceModel.load();

    setState(() {});

    testResultModel = TestResultModel(
        start: DateTime.now().toString(),
        children: List<TestResultCaseModel>());

    engine = CommandEngine(
        testCaseModel: testCaseModel,
        testResourceModel: testResourceModel,
        testResultModel: testResultModel,
        onStatusUpdate: () {
          setState(() {});
        },)
      ..assemble()
      ..next();
  }

  TestResultItemModel playItem(FlutterCicadaPlayer player, String resId) {
    print('id = ${resId}');
    TestResourceItemModel model = testResourceModel.findItemById(resId);
    print('url = ${model.url}');
    player.setUrl(model.url);
    player.prepare();

    TestResultItemModel resultItem =
        TestResultItemModel(id: resId, start: DateTime.now().toString());
    return resultItem;
  }

  void close(FlutterCicadaPlayer player) {
    player.stop();
    player.destroy();

    testResultModel.saveToFile();
  }
}
