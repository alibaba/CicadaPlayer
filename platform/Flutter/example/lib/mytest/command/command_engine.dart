import 'package:flutter/foundation.dart';
import 'package:flutter_cicadaplayer/flutter_cicadaplayer.dart';
import 'package:flutter_cicadaplayer/flutter_cicadaplayer_factory.dart';
import 'package:flutter_cicadaplayer_example/mytest/command/command.dart';
import 'package:flutter_cicadaplayer_example/mytest/command/command_regist.dart';
import 'package:flutter_cicadaplayer_example/mytest/test_case_model.dart';
import 'package:flutter_cicadaplayer_example/mytest/test_resource_model.dart';
import 'package:flutter_cicadaplayer_example/mytest/test_result_model.dart';
import 'package:fluttertoast/fluttertoast.dart';

typedef OnStatusUpdate = void Function();

class CommandEngine with CommandRegist{

  TestResultModel testResultModel;
  TestResourceModel testResourceModel;
  TestCaseModel testCaseModel;

  OnStatusUpdate onStatusUpdate;

  int curIdx = -1;

  int resIdx = 0;

  FlutterCicadaPlayer fCicadaPlayer;

  CommandEngine({@required this.testCaseModel,@required this.testResourceModel,@required this.testResultModel,this.onStatusUpdate});

  void next(){
    curIdx++;
    if(curIdx>=commandList.length){
      Fluttertoast.showToast(msg: "全部用例完成");
    }else{
      run(commandList[curIdx]);
    }
  }

  void run(Command command) {
    List<TestResultItemModel> children = new List<TestResultItemModel>();
    TestResultCaseModel resultCase =
        TestResultCaseModel(name: command.caseName, children: children);
    testResultModel.children.add(resultCase);

    fCicadaPlayer =
        FlutterCicadaPlayerFactory().createCicadaPlayer();
    fCicadaPlayer.setAutoPlay(true);
    resIdx = 0;

    TestCaseGroup testCaseGroup = testCaseModel.findCaseGroupByName(command.groupName);
    command.initCommand(testCaseGroup);
    TestCaseItem caseItem = testCaseGroup.findCaseItemByName(command.caseName);
    statusUpdate(caseItem,TestCaseItem.TestCaseItemDoing);
    command.handleCommand(fCicadaPlayer,caseItem,resultCase,this);

    play(fCicadaPlayer, caseItem, resultCase);
  }

  void play(FlutterCicadaPlayer player,TestCaseItem caseItem,TestResultCaseModel resultCase){
    String resId = caseItem.resources[resIdx];
    resultCase.children.add(playItem(player, resId));
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

  void statusUpdate(TestCaseItem caseItem,int status){
    caseItem.status = status;
    if(this.onStatusUpdate!=null){
      this.onStatusUpdate();
    }
  }

  void resultErr(TestResultCaseModel resultCase,TestCaseItem caseItem,String desc){
    TestResultItemModel model = resultCase.children.last;
      model.finish(
          result: 'failed',
          description:desc);
    statusUpdate(caseItem,TestCaseItem.TestCaseItemErr);
  }

  void resultOK(TestResultCaseModel resultCase,TestCaseItem caseItem){
      TestResultItemModel model = resultCase.children.last;
      model.finish(result: 'ok', description: 'ok');
      statusUpdate(caseItem,TestCaseItem.TestCaseItemOK);
  }

  void close(FlutterCicadaPlayer player,Command command) {
    player.stop();
    player.destroy();
    player = null;

    command.destroyCommand();

    testResultModel.saveToFile();

    next();
  }

  void clear(){
    if(fCicadaPlayer!=null){
      fCicadaPlayer.stop();
      fCicadaPlayer.destroy();
    }
  }

}
