import 'package:flutter/material.dart';

typedef OnSelectAtIdx = void Function(int value);

class CicadaSegment extends StatelessWidget {
  final List<String> titles;
  final int selIdx;
  final OnSelectAtIdx onSelectAtIdx;

  const CicadaSegment(
      {Key key, this.titles, this.selIdx = 0, this.onSelectAtIdx})
      : super(key: key);

  @override
  Widget build(BuildContext context) {
    return Row(
        children: titles
            .asMap()
            .map((i, value) => MapEntry(
                i,
                Expanded(
                    child: i == selIdx
                        ? FlatButton(
                            color: Theme.of(context).accentColor,
                            textColor: Theme.of(context).canvasColor,
                            shape: RoundedRectangleBorder(),
                            onPressed: () => _onSelIdx(i),
                            child: Text(
                              value,
                              maxLines: 1,
                              overflow: TextOverflow.ellipsis,
                            ))
                        : OutlineButton(
                            color: Theme.of(context).accentColor,
                            textColor: Theme.of(context).accentColor,
                            shape: RoundedRectangleBorder(),
                            onPressed: () => _onSelIdx(i),
                            child: Text(
                              value,
                              maxLines: 1,
                              overflow: TextOverflow.ellipsis,
                            )))))
            .values
            .toList());
  }

  _onSelIdx(int idx) {
    if (onSelectAtIdx != null) {
      onSelectAtIdx(idx);
    }
  }
}
