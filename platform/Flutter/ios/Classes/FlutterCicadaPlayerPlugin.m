#import "FlutterCicadaPlayerPlugin.h"
#import "CicadaPlayerFactory.h"

@implementation FlutterCicadaPlayerPlugin
//+ (void)registerWithRegistrar:(NSObject<FlutterPluginRegistrar>*)registrar {
//  FlutterMethodChannel* channel = [FlutterMethodChannel
//      methodChannelWithName:@"flutter_aliplayer"
//            binaryMessenger:[registrar messenger]];
//  FlutterAliplayerPlugin* instance = [[FlutterAliplayerPlugin alloc] init];
//  [registrar addMethodCallDelegate:instance channel:channel];
//}

+ (void)registerWithRegistrar:(NSObject<FlutterPluginRegistrar> *)registrar
{
    CicadaPlayerFactory *factory = [[CicadaPlayerFactory alloc] initWithMessenger:registrar.messenger];
    [registrar registerViewFactory:factory withId:@"plugins.flutter_cicadaplayer"];
}


- (void)handleMethodCall:(FlutterMethodCall *)call result:(FlutterResult)result
{
    if ([@"getPlatformVersion" isEqualToString:call.method]) {
        result([@"iOS " stringByAppendingString:[[UIDevice currentDevice] systemVersion]]);
    } else {
        result(FlutterMethodNotImplemented);
    }
}

@end
