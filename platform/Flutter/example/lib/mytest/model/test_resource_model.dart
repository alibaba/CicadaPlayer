import 'package:flutter/services.dart';
import 'package:xml_parser/xml_parser.dart';

class TestResourceModel{
  Map<String,TestResourceItemModel> contentMap;

  Future<void> load() async{
    contentMap = Map<String,TestResourceItemModel>();
    String sourceContents = await rootBundle.loadString("assets/xml/test_resources.xml");
    XmlDocument sourceXmlDocument = XmlDocument.fromString(sourceContents);
    XmlElement sourceRootNode = sourceXmlDocument.getElement('test_resources');

    for (XmlElement item in sourceRootNode.children) {
      TestResourceItemModel model = TestResourceItemModel(
        id: item.id,
        url: item.getLastElementWhere(name: 'url').text,
        duration:int.tryParse(item.getLastElementWhere(name: 'duration').text),
        container: item.getLastElementWhere(name: 'container').text,
        protocol: item.getLastElementWhere(name: 'protocol').text,
        video: item.getLastElementWhere(name: 'video').text,
        audio: item.getLastElementWhere(name: 'audio').text,
        );
      contentMap[item.id] = model;
    }

  }

  TestResourceItemModel findItemById(String id){
    return contentMap[id];
  }
  
}

class TestResourceItemModel{
  String id;
  String url;
  int duration;
  String container;
  String protocol;
  String video;
  String audio;

  TestResourceItemModel({this.id,this.url,this.duration=-1,this.container,this.protocol,this.video,this.audio});

}