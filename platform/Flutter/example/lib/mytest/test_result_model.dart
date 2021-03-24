import 'dart:io';

import 'package:path_provider/path_provider.dart';
import 'package:xml_parser/xml_parser.dart';

class TestResultModel {
  String start;
  int duration;
  int count;
  int fail;
  List<TestResultCaseModel> children;

  TestResultModel(
      {this.start,
      this.duration = 0,
      this.count = 0,
      this.fail = 0,
      this.children});

  void saveToFile() async {
    List<XmlElement> cases = List<XmlElement>();
    for (TestResultCaseModel item in children) {
      cases.add(item.toXml());
    }

    XmlDocument xmlDocument = XmlDocument([
      XmlDeclaration(version: '1.0', encoding: 'UTF-8'),
      XmlElement(
          name: 'test_result',
          attributes: [
            XmlAttribute('start', start),
            XmlAttribute('duration', '$duration'),
            XmlAttribute('count', '$count'),
            XmlAttribute('fail', '$fail'),
          ],
          children: cases),
    ]);

    final appDocDir = Platform.isIOS?await getApplicationDocumentsDirectory():await getExternalStorageDirectory();
    var dir = Directory(appDocDir.path + "/" + "test_result");
    try {
      bool exists = await dir.exists();
      if (!exists) {
        await dir.create();
      }
    } catch (e) {
      print(e);
    }

    File file = File("${dir.path}/test_result_$start.xml");
    bool exists = await file.exists();
    if (!exists) {
      await file.create();
    }
    file.writeAsString(xmlDocument.toFormattedString());
  }
}

class TestResultCaseModel {
  String name;
  List<TestResultItemModel> children;
  TestResultCaseModel({this.name, this.children});

  XmlElement toXml() {
    List<XmlElement> resurces = List<XmlElement>();
    for (TestResultItemModel item in children) {
      resurces.add(item.toXml());
    }
    return XmlElement(
        name: 'case',
        attributes: [XmlAttribute('name', name)],
        children: resurces);
  }
}

class TestResultItemModel {
  String id;
  String start;
  int duration;
  String result;
  String description;

  TestResultItemModel(
      {this.id,
      this.start,
      this.duration = -1,
      this.result = '',
      this.description = ''});

  XmlElement toXml() {
    return XmlElement(name: 'resurce', children: [
      XmlElement(name: 'id', children: [XmlText(id)]),
      XmlElement(name: 'start', children: [XmlText(start)]),
      XmlElement(name: 'duration', children: [XmlText('$duration')]),
      XmlElement(name: 'result', children: [XmlText(result)]),
      XmlElement(name: 'description', children: [XmlText(description)]),
    ]);
  }

  void finish({String result, String description}) {
    DateTime startTime = DateTime.parse(start);
    duration = DateTime.now().difference(startTime).inSeconds;
    this.result = result;
    this.description = description;
  }
}
