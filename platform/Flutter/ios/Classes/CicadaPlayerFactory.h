//
//  VideoViewFactory.h
//  flutter_cicadaplayer
//
//  Created by aliyun on 2020/10/9.
//
#import <CicadaPlayerSDK/CicadaPlayer.h>
#import <Flutter/Flutter.h>
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface CicadaPlayerFactory : NSObject <FlutterPlatformViewFactory, CicadaDelegate, CicadaAudioSessionDelegate, FlutterStreamHandler>

@property(nonatomic, strong, nullable) CicadaPlayer *cicadaPlayer;

- (instancetype)initWithMessenger:(NSObject<FlutterBinaryMessenger> *)messenger;

@end

NS_ASSUME_NONNULL_END
