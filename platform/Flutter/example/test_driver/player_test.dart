import 'dart:io';

import 'package:flutter_driver/flutter_driver.dart';
import 'package:test/test.dart';
import 'package:xml_parser/xml_parser.dart';

void main() {
  group('TestPlayer', () {
    // First, define the Finders and use them to locate widgets from the
    // test suite. Note: the Strings provided to the `byValueKey` method must
    // be the same as the Strings we used for the Keys in step 1.
    final prepareBtnFinder = find.byValueKey('prepare');
    final playBtnFinder = find.byValueKey('play');
    final stopBtnFinder = find.byValueKey('stop');
    final urlFieldFinder = find.byValueKey('urlField');
    final urlBtnFinder = find.byValueKey('urlBtn');
    final resultTextFinder = find.byValueKey('resultText');

    FlutterDriver driver;
    XmlElement rootNode;
    XmlElement sourceRootNode;

    // Connect to the Flutter driver before running any tests.
    setUpAll(() async {
      driver = await FlutterDriver.connect();

      File file=new File("test_driver/xml/test_case.xml");
      String contents = await file.readAsString();
      XmlDocument xmlDocument = XmlDocument.fromString(contents);
      rootNode = xmlDocument.getElement('test');

      File sourceFile=new File("test_driver/xml/test_resources.xml");
      String sourceContents = await sourceFile.readAsString();
      XmlDocument sourceXmlDocument = XmlDocument.fromString(sourceContents);
      sourceRootNode = sourceXmlDocument.getElement('test_resources');

    });

    // Close the connection to the driver after the tests have completed.
    tearDownAll(() async {
      if (driver != null) {
        driver.close();
      }
    });

    test('format', () async {
      final casegroups = rootNode.getElementsWhere(
        name: 'casegroup',
        attributes: [XmlAttribute('casegroup_name', 'format')],
        );
      final resources = casegroups.first.getElement('resources').getChildren('id');
      for (var item in resources) {
        print('id = ${item.text}');
        final resource = sourceRootNode.getLastElementWhere(
        id: item.text,
        );
        String url = resource.getChild('url').text;
        print('url = $url');

        await driver.tap(urlFieldFinder);  // acquire focus
        await driver.enterText(url);  // enter text
        await driver.waitFor(find.text(url)); 
        await driver.tap(urlBtnFinder);
        await driver.tap(prepareBtnFinder);
        await driver.waitFor(find.text('准备完成'));
        expect(await driver.getText(resultTextFinder), "准备完成");
        await driver.tap(playBtnFinder);
        await driver.waitFor(find.text('首帧渲染'));
        expect(await driver.getText(resultTextFinder), "首帧渲染");

        // driver.waitUntilNoTransientCallbacks(timeout: Duration(seconds: 5*60)).then((value){
        //   driver.tap(stopBtnFinder);
        // });

        Future.delayed(Duration(seconds: 5*60),(){
            driver.tap(stopBtnFinder);
	      });

        await driver.waitFor(find.text('播放停止'));
        expect(await driver.getText(resultTextFinder), "播放停止");

      }
    });

  });
}