import 'package:flutter/services.dart';
import 'package:xml_parser/xml_parser.dart';

class TestCaseModel{
  bool stopOnError;
  bool run;

  Map<String,TestCaseGroup> contentMap = {};

  Future<void> load() async{
    String sourceContents = await rootBundle.loadString("assets/xml/test_case.xml");
    XmlDocument sourceXmlDocument = XmlDocument.fromString(sourceContents);
    XmlElement sourceRootNode = sourceXmlDocument.getElement('test');

    for (XmlElement item in sourceRootNode.children) {
      TestCaseGroup model = TestCaseGroup.modelFromXml(item);
      contentMap[model.name] = model;
    }

  }

  TestCaseGroup findCaseGroupByName(String caseGroupName){
    return contentMap[caseGroupName];
  }
  
}

class TestCaseGroup{
  String name;
  List<TestCaseItem> children = [];
  XmlElement element;
  bool isExpanded;

  TestCaseGroup({this.name,this.children,this.element,this.isExpanded=true});

  static TestCaseGroup modelFromXml(XmlElement xml){
    TestCaseGroup model = TestCaseGroup(name: xml.getAttribute('casegroup_name'),element: xml,children:[] );
    for (XmlElement item in xml.children) {
      model.children.add(TestCaseItem.modelFromXml(item));
    }
    return model;
  }

  TestCaseItem findCaseItemByName(String caseItemName){
    return children.firstWhere((element) => element.name==caseItemName);
  }

}

class TestCaseItem{

  static const int  TestCaseItemUnDo = 0;
  static const int TestCaseItemDoing = 1;
  static const int TestCaseItemOK = 2;
  static const int TestCaseItemErr = 3;

  static const List<String> StatusList = ['未执行','未执中','成功','失败'];

  String name;
  List<String> resources = [];
  XmlElement element;
  TestCaseItem({this.name,this.resources,this.element,this.status=TestCaseItemUnDo});
  //0 未执行 1执行中 2 成功 3 失败
  int status;

  String getStatusStr(){
    return TestCaseItem.StatusList[status];
  }

  static TestCaseItem modelFromXml(XmlElement xml){
    TestCaseItem model = TestCaseItem(
      name: xml.getAttribute('case_name'),
      resources: [],
      element:xml
      );
      XmlElement resources = xml.getElementWhere(name: 'resources');
      for (XmlElement item in resources.children) {
        model.resources.add(item.text);
      }
      return model;
  }
}