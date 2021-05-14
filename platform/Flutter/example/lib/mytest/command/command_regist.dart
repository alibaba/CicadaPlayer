import 'package:flutter_cicadaplayer_example/mytest/command/command.dart';
import 'package:flutter_cicadaplayer_example/mytest/command/format_command.dart';
import 'package:flutter_cicadaplayer_example/mytest/command/seek_command.dart';

class CommandRegist{
  List<Command> commandList = [];

  void assemble(){
    /// casegroup format
    commandList.add(FormatCommand('format','format_http_mp4_h264_aac'));

    /// casegroup seek
    commandList.add(SeekCommand('operation_seek','operation_seek_one_by_one'));
    commandList.add(SeekCommand('operation_seek','operation_seek_one_by_one_accurate'));

    commandList.add(SeekCommand('operation_seek','operation_seek_by_time'));
    commandList.add(SeekCommand('operation_seek','operation_seek_by_time_accurate'));

    // commandList.add(SeekCommand('operation_seek','operation_seek_preview'));
    // commandList.add(SeekCommand('operation_seek','operation_seek_preview_accurate'));

    commandList.add(SeekCommand('operation_seek','operation_seek_to_end'));
    commandList.add(SeekCommand('operation_seek','operation_seek_to_end_loop'));

  }
}