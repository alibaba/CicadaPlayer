import 'package:flutter/material.dart';
import 'package:flutter_cicadaplayer_example/model/video_model.dart';

class PlayerFlagment extends StatefulWidget {
  final VideoModel model;

  const PlayerFlagment({Key key, this.model}) : super(key: key);
  @override
  _PlayerFlagmentState createState() => _PlayerFlagmentState();
}

class _PlayerFlagmentState extends State<PlayerFlagment> {
  @override
  Widget build(BuildContext context) {
    return Stack(
      children: [
        Image.network(
          widget.model.coverUrl,
          fit: BoxFit.fitWidth,
        )
      ],
    );
  }
}
