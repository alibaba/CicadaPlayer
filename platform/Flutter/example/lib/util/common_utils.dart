import 'package:flutter/material.dart';

class CommomUtils {
  static pushPage(BuildContext context, Widget route) {
    Navigator.push(context, MaterialPageRoute(builder: (context) => route));
  }
}
