import 'package:flutter_cicadaplayer/flutter_cicadaplayer.dart';
import 'package:flutter_cicadaplayer_example/mytest/command/command_engine.dart';
import 'package:flutter_cicadaplayer_example/mytest/test_case_model.dart';
import 'package:flutter_cicadaplayer_example/mytest/test_result_model.dart';

class Command{
  String groupName;
  String caseName;

  Command(this.groupName,this.caseName);

  void initCommand(TestCaseGroup testCaseGroup){}

  void handleCommand(FlutterCicadaPlayer player,TestCaseItem caseItem,TestResultCaseModel resultCase,CommandEngine engine){}

  void destroyCommand(){}

}