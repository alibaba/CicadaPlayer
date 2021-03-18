import 'dart:io';
import 'package:flutter_test/flutter_test.dart';

import 'package:xml_parser/xml_parser.dart';

void main() {
  
  test('xml', () async {
    // Verify that platform version is retrieved.
    File file=new File("test_driver/xml/test_case.xml");
    String contents = await file.readAsString();
    XmlDocument xmlDocument = XmlDocument.fromString(contents);
    final rootNode = xmlDocument.getElement('test');
    // final casegroups = rootNode.getElementsWhere(name: 'casegroup');
    // final author = casegroups.first.getAttribute("casegroup_name");
    // print(author);
    // expect(author,'format');

     final casegroups = rootNode.getElementsWhere(
        name: 'casegroup',
        attributes: [XmlAttribute('casegroup_name', 'format')],
        );
      final resources = casegroups.first.getElement('resources').getChildren('id');
      for (var item in resources) {
        print(item.text);
      }
      expect(resources.length>0,true);

  });

// <test_resources>
//     <resources>
//         <id>http_mp4_h264_aac</id>
//         <url>http://player.alicdn.com/video/aliyunmedia.mp4</url>
//         <duration>00:04:22.08</duration> <!-- 时长 格式?-->
//         <container>mp4</container>
//         <protocol>http</protocol>
//         <video>h264</video> <!-- 视频流的codec -->
//         <audio>aac</audio> <!-- 音频流的codec -->
//     </resources>
//     <resources>
//         <id>https_mp4_h264_aac</id>
//         <url>https://player.alicdn.com/video/aliyunmedia.mp4</url>
//         <duration>00:04:22.08</duration>
//         <container>mp4</container>
//         <protocol>https</protocol>
//         <video>h264</video>
//         <audio>aac</audio>
//     </resources>
// </test_resources>

  test('resource', () async {
    // Verify that platform version is retrieved.
    File file=new File("test_driver/xml/test_resources.xml");
    String contents = await file.readAsString();
    XmlDocument xmlDocument = XmlDocument.fromString(contents);
    final rootNode = xmlDocument.getElement('test_resources');

     final resources = rootNode.getLastElementWhere(
        id: 'https_mp4_h264_aac',
        );
      // final resource = resources.first.getChild('id');
      print(resources.getChild('url').text);
      expect(resources==null,false);

  });

}