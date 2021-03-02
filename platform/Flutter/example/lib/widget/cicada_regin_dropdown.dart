import 'package:flutter/material.dart';

typedef OnRegionChanged = void Function(String region);

class ReginDropDownButton extends StatefulWidget {
  OnRegionChanged onRegionChanged;
  String currentHint = "cn-shanghai";

  ReginDropDownButton({Key key, this.onRegionChanged, this.currentHint})
      : super(key: key);

  @override
  _ReginDropDownButtonState createState() => _ReginDropDownButtonState();
}

class _ReginDropDownButtonState extends State<ReginDropDownButton> {
  List<DropdownMenuItem> getItemList() {
    List<DropdownMenuItem> items = List();
    DropdownMenuItem item1 = DropdownMenuItem(
      child: Text("cn-shanghai"),
      value: "cn-shanghai",
    );
    DropdownMenuItem item2 = DropdownMenuItem(
      child: Text("ap-southeast-1"),
      value: "ap-southeast-1",
    );
    DropdownMenuItem item3 = DropdownMenuItem(
      child: Text("ap-northeast-1"),
      value: "ap-northeast-1",
    );
    DropdownMenuItem item4 = DropdownMenuItem(
      child: Text("cn-beijing"),
      value: "cn-beijing",
    );
    items.add(item1);
    items.add(item2);
    items.add(item3);
    items.add(item4);
    return items;
  }

  @override
  Widget build(BuildContext context) {
    return DropdownButton(
        items: getItemList(),
        isExpanded: true,
        hint: Text(widget.currentHint),
        onChanged: (value) {
          setState(() {
            widget.currentHint = value;
            if (widget.onRegionChanged != null) {
              widget.onRegionChanged(value);
            }
          });
        });
  }
}
