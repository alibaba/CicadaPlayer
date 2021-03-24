// <?xml version="1.0" encoding="utf-8"?>
// <test_resources>
//     <resource id='http_mp4_h264_aac'>
//         <url>http://player.alicdn.com/video/aliyunmedia.mp4</url>
//         <duration>00:04:22.08</duration> <!-- 时长 格式?-->
//         <container>mp4</container>
//         <protocol>http</protocol>
//         <video>h264</video> <!-- 视频流的codec -->
//         <audio>aac</audio> <!-- 音频流的codec -->
//     </resource>
//     <resource id='https_mp4_h264_aac'>
//         <url>https://alivc-demo-vod.aliyuncs.com/b022c7023e824729aa466b51a3c6334d/34ffb378972247509e32cd9c1e8f87c6-4f37beaa1dffcc52acfcc0b4b6451345-ld.mp4</url>
//         <duration>00:04:22.08</duration>
//         <container>mp4</container>
//         <protocol>https</protocol>
//         <video>h264</video>
//         <audio>aac</audio>
//     </resource>
// </test_resources>

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