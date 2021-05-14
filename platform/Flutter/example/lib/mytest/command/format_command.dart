import 'package:flutter_cicadaplayer/flutter_cicadaplayer.dart';
import 'package:flutter_cicadaplayer_example/mytest/command/command.dart';
import 'package:flutter_cicadaplayer_example/mytest/model/test_case_model.dart';
import 'package:flutter_cicadaplayer_example/mytest/model/test_result_model.dart';
import 'command_engine.dart';

class FormatCommand extends Command{

  FormatCommand(String groupName, String caseName) : super(groupName, caseName);

  @override
  void handleCommand(FlutterCicadaPlayer player,TestCaseItem caseItem,TestResultCaseModel resultCase,CommandEngine engine) {
    player.setOnCompletion(() {
      engine.resultOK(resultCase,caseItem);
      engine.resIdx++;
      if (engine.resIdx < caseItem.resources.length) {
        player.stop();
        engine.play(player, caseItem, resultCase);
      } else {
        //完成
        engine.close(player,this);
      }
    });

    player.setOnError((errorCode, errorExtra, errorMsg) {
      engine.resultErr(resultCase,caseItem,'errorCode=$errorCode,errorExtra=$errorExtra,errorMsg=$errorMsg');
      engine.close(player,this);
    });
  }
}