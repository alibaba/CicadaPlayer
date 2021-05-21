import 'dart:async';
import 'dart:math';

import 'package:flutter_cicadaplayer/flutter_cicadaplayer.dart';
import 'package:flutter_cicadaplayer_example/mytest/command/command.dart';
import 'package:flutter_cicadaplayer_example/mytest/model/test_case_model.dart';
import 'package:flutter_cicadaplayer_example/mytest/model/test_result_model.dart';

import 'command_engine.dart';

class SeekCommand extends Command {
  bool accurate;
  bool preview;
  int interval;
  int time;
  bool loop;
  List positionList = [];

  Timer timer;
  int seekPos = -1;
  int dration;

  SeekCommand(String groupName, String caseName) : super(groupName, caseName);

  @override
  void initCommand(TestCaseGroup testCaseGroup) {
    if (testCaseGroup.element.hasAttribute('accurate')) {
      accurate = testCaseGroup.element.getAttribute('accurate') == 'true';
    }
    if (testCaseGroup.element.hasAttribute('preview')) {
      preview = testCaseGroup.element.getAttribute('preview') == 'true';
    }
    if (testCaseGroup.element.hasAttribute('time')) {
      time = int.tryParse(testCaseGroup.element.getAttribute('time'));
    }
    if (testCaseGroup.element.hasAttribute('loop')) {
      loop = testCaseGroup.element.getAttribute('loop') == 'true';
    }
    if (testCaseGroup.element.hasAttribute('interval')) {
      interval = int.tryParse(testCaseGroup.element.getAttribute('time'));
    }
  }

  @override
  void handleCommand(FlutterCicadaPlayer player, TestCaseItem caseItem,
      TestResultCaseModel resultCase, CommandEngine engine) {
    if (caseItem.element.hasChild('accurate')) {
      accurate =
          caseItem.element.getChildWhere(name: 'accurate').text == 'true';
    }
    if (caseItem.element.hasChild('loop')) {
      loop = caseItem.element.getChildWhere(name: 'loop').text == 'true';
    }
    if (caseItem.element.hasChild('preview')) {
      preview = caseItem.element.getChildWhere(name: 'preview').text == 'true';
    }
    if (caseItem.element.hasChild('time')) {
      time = int.tryParse(caseItem.element.getChildWhere(name: 'time').text);
    }
    if (caseItem.element.hasChild('interval')) {
      interval =
          int.tryParse(caseItem.element.getChildWhere(name: 'interval').text);
    }
    if (caseItem.element.hasChild('position')) {
      List list =caseItem.element.getChild('position').children;
      for (var item in list) {
        positionList.add(item.text);
      }
      time = time *list.length;
    }

    player.setLoop(loop);

    player.setOnPrepared(() async {
      dration = await player.getDuration();

      if (time > 0) {
        if (interval > 0) {
          timer = Timer.periodic(Duration(seconds: interval), (timer) {
            ///定时任务
            _seek(player);
            //结束定时任务
            if (time <= 0) {
              engine.resultOK(resultCase,caseItem);
              timer.cancel();
              engine.close(player, this);
            }
          });
        } else {
          _seek(player);
        }
      }
    });

    bool hasSeek= false;
    player.setOnSeekComplete(() {
      hasSeek = true;
    });

    player.setOnInfo((infoCode, extraValue, extraMsg) {
      if (hasSeek && infoCode == FlutterAvpdef.CURRENTPOSITION) {
        //误差在20s内为正常
        if(interval==-1){//one_by_one模式
          if (seekPos > 0 && (extraValue - seekPos).abs() > 20*1000) {
            print('seek与实际值相差超过20s:seekPos=$seekPos,curPos=$extraValue');
            engine.resultErr(resultCase,caseItem, 'seek与实际值相差超过20s:seekPos=$seekPos,curPos=$extraValue');
            engine.close(player, this);
          }else{
            hasSeek= false;
            _seek(player);
            if (time <= 0) {
              engine.resultOK(resultCase,caseItem);
              engine.close(player, this);
            }
          }
        }
        
      }
    });

    player.setOnError((errorCode, errorExtra, errorMsg) {
      engine.resultErr(resultCase,caseItem,
          'errorCode=$errorCode,errorExtra=$errorExtra,errorMsg=$errorMsg');
      engine.close(player, this);
    });
  }

  @override
  void destroyCommand() {
    timer?.cancel();
  }

  /// 私有方法
  void _seek(FlutterCicadaPlayer player){
    if(positionList.length>0){
      int postionIdx = time%positionList.length;
      String text = positionList[postionIdx];
      if(text.contains('end')){
        text = text.replaceAll('end', '');
        seekPos = dration + (int.tryParse(text)??0);
      }
    }else{
      seekPos = Random().nextInt(dration);
    }
    player.seekTo(seekPos,
              accurate ? FlutterAvpdef.ACCURATE : FlutterAvpdef.INACCURATE);
    time--;
  }
}
