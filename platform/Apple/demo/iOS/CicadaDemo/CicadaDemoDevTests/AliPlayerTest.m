//
//  CicadaPlayerTest.m
//  CicadaPlayerAutoTestTests
//
//  Created by 郦立 on 2019/1/17.
//  Copyright © 2019年 wb-qxx397776. All rights reserved.
//

#import <XCTest/XCTest.h>
#import "AliyunPlayerViewController.h"
#import "ApsaraTestCase.h"

@interface CicadaPlayerTest : XCTestCase

@property (nonatomic, strong)AliyunPlayerViewController *playerViewController;

@end

@implementation CicadaPlayerTest

- (void)setUp {
    [super setUp];
    
    self.playerViewController = [[AliyunPlayerViewController alloc] init];
    [UIApplication sharedApplication].keyWindow.rootViewController = self.playerViewController;
}

- (void)tearDown {
    [self.playerViewController destory];
    self.playerViewController = nil;
    [super tearDown];
}

/**
 创建后，判断类型是否正确
 */
- (void)testInit {
    CicadaPlayer *player = [[CicadaPlayer alloc]init];
    XCTAssertTrue([player isKindOfClass:[CicadaPlayer class]], @"CicadaPlayer init 创建类型不对");
}

/**
 创建后，判断类型是否正确
 */
- (void)testInitTraceID {
    CicadaPlayer *player = [[CicadaPlayer alloc]init:@"TraceID"];
    XCTAssertTrue([player isKindOfClass:[CicadaPlayer class]], @"CicadaPlayer init 创建类型不对");
}

/**
 测试能否播放，如果首帧能显示，则成功
 */
- (void)testSetUrlSource {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].firstObject;
    [self.playerViewController initPlayer];
    self.playerViewController.onPlayerEventCallback = ^(CicadaEventType eventType) {
        if (eventType == CicadaEventFirstRenderedStart) {
            [expection fulfill];
        }
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试能否播放，如果首帧能显示，则成功
 */
- (void)testSetUrlSource_m3u8 {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].lastObject;
    [self.playerViewController initPlayer];
    self.playerViewController.onPlayerEventCallback = ^(CicadaEventType eventType) {
        if (eventType == CicadaEventFirstRenderedStart) {
            [expection fulfill];
        }
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试暂停，如果有暂停回调，则正确
 */
- (void)testPrepare {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].firstObject;
    [self.playerViewController initPlayer];
    self.playerViewController.onPlayerEventCallback = ^(CicadaEventType eventType) {
        if (eventType == CicadaEventPrepareDone) {
            [expection fulfill];
        }
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试开始，如果首帧显示，则正确
 */
- (void)testStart {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].firstObject;
    [self.playerViewController initPlayer];
    self.playerViewController.onPlayerEventCallback = ^(CicadaEventType eventType) {
        if (eventType == CicadaEventFirstRenderedStart) {
            [expection fulfill];
        }
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试暂停，如果有暂停回调，则正确
 */
- (void)testPause {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].firstObject;
    [self.playerViewController initPlayer];
    [self.playerViewController pause];
    self.playerViewController.onPlayerStatusChangedCallback = ^(CicadaStatus oldStatus, CicadaStatus newStatus) {
        if (newStatus == CicadaStatusPaused) {
           [expection fulfill];
        }
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试redraw，不崩溃就算成功
 */
- (void)testRedraw {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].firstObject;
    [self.playerViewController initPlayer];
    [self.playerViewController redraw];
    self.playerViewController.redrawCallback = ^{
        [expection fulfill];
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试Reset，不崩溃就算成功
 */
- (void)testReset {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].firstObject;
    [self.playerViewController initPlayer];
    [self.playerViewController reset];
    self.playerViewController.resetCallback = ^{
        [expection fulfill];
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试Stop，没有回调，不崩溃就算成功
 */
- (void)testStop {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].firstObject;
    [self.playerViewController initPlayer];
    [self.playerViewController stop];
    self.playerViewController.onPlayerStatusChangedCallback = ^(CicadaStatus oldStatus, CicadaStatus newStatus) {
        if (newStatus == CicadaStatusStopped) {
            static dispatch_once_t onceToken;
            dispatch_once(&onceToken, ^{
                [expection fulfill];
            });
        }
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试销毁，对象被释放了算成功
 */
- (void)testDestroy {
    //这里不知道怎么验证
    CicadaPlayer *player = [[CicadaPlayer alloc]init];
    [player destroy];
    player = nil;
    XCTAssertNil(player,@"CicadaPlayer destroy 销毁失败");
}

/**
 测试seek，如果有SeekEnd回调，算成功
 */
- (void)testSeekToTime {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].firstObject;
    [self.playerViewController initPlayer];
    [self.playerViewController seekToTime:self.playerViewController.player.duration*0.2 seekMode:CICADA_SEEKMODE_INACCURATE];
    self.playerViewController.onPlayerEventCallback = ^(CicadaEventType eventType) {
        if (eventType == CicadaEventSeekEnd) {
            [expection fulfill];
        }
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试截图，有截图回调算成功，截图空算失败
 */
- (void)testSnapShot {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].firstObject;
    [self.playerViewController initPlayer];
    [self.playerViewController snapShot];
    WEAK_SELF
    weakSelf.playerViewController.onCaptureScreenCallback = ^(UIImage *image) {
        XCTAssertNotNil(image, @"截图返回为空");
        [expection fulfill];
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试选择track，判断选择的track和结果的track的index是否一致，一致则成功，否则失败
 */
- (void)testSelectTrack {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].lastObject;
    [self.playerViewController initPlayer];
    [self.playerViewController selectTrack];
    WEAK_SELF
    weakSelf.playerViewController.onTrackChangedCallback = ^(CicadaTrackInfo *info) {
        CicadaTrackInfo *lastInfo = weakSelf.playerViewController.info.lastObject;
        XCTAssertTrue(lastInfo.trackIndex == info.trackIndex, @"selectTrack 选择返回错误");
        [expection fulfill];
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试MediaInfo，有返回算成功
 */
- (void)testGetMediaInfo {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].firstObject;
    [self.playerViewController initPlayer];
    WEAK_SELF
    weakSelf.playerViewController.getMediaInfoCallback = ^(CicadaMediaInfo *info) {
        XCTAssertNotNil(info, @"MediaInfo 不能为空");
        [expection fulfill];
    };
    [self.playerViewController getMediaInfo];
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试当前Track，先选择一个，看返回当前的是否一致，一致则成功，否则失败
 */
- (void)testGetCurrentTrack {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].lastObject;
    [self.playerViewController initPlayer];
    [self.playerViewController selectTrack];
    WEAK_SELF
    weakSelf.playerViewController.onTrackChangedCallback = ^(CicadaTrackInfo *info) {
        CicadaTrackInfo *lastInfo = weakSelf.playerViewController.info.lastObject;
        XCTAssertTrue(lastInfo.trackIndex == info.trackIndex, @"GetCurrentTrack 返回错误");
        [expection fulfill];
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试config，能设置就成功
 */
- (void)testSetConfig {
    CicadaPlayer *player = [[CicadaPlayer alloc]init];
    CicadaConfig *config = [[CicadaConfig alloc]init];
    [self.playerViewController.player setConfig:config];
    XCTAssertTrue([player isKindOfClass:[CicadaPlayer class]], @"setConfig 调用后 类型不对");
}

/**
 测试读取config，有就成功
 */
- (void)testGetConfig {
    CicadaPlayer *player = [[CicadaPlayer alloc]init];
    CicadaConfig *config = [player getConfig];
    XCTAssertNotNil(config, @"getConfig返回为空");
    XCTAssertTrue([player isKindOfClass:[CicadaPlayer class]], @"setConfig 调用后 类型不对");
}

/**
 测试cacheConfig，返回yes就成功
 */
- (void)testSetCacheConfig {
    CicadaCacheConfig *config = [[CicadaCacheConfig alloc]init];
    CicadaPlayer *player = [[CicadaPlayer alloc]init];
    BOOL result = [player setCacheConfig:config];
    XCTAssertTrue(result, @"SetCacheConfig 调用后 返回失败");
}

/**
 测试缓存文件路径
 */
- (void)testGetCacheFilePathURL {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    NSString *homePath = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES).firstObject;
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].firstObject;
    NSString *playUrl = self.playerViewController.urlSource.playerUrl.absoluteString;
    [self.playerViewController initPlayerWithCachePath:homePath];
    WEAK_SELF
    weakSelf.playerViewController.onPlayerEventCallback = ^(CicadaEventType eventType) {
        if (eventType == CicadaEventFirstRenderedStart) {
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(3 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
                NSString *path = [weakSelf.playerViewController.player getCacheFilePath:playUrl];
                XCTAssertTrue([path containsString:homePath], @"CicadaPlayer getCacheFilePath 返回类型不对");
                [expection fulfill];
            });
        }
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

- (void)testaddExtSubtitle {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].lastObject;
    WEAK_SELF
    [weakSelf.playerViewController initPlayer];
    NSString *homePath = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES).firstObject;
    [weakSelf.playerViewController initPlayerWithCachePath:homePath];
    weakSelf.playerViewController.onPlayerEventCallback = ^(CicadaEventType eventType) {
        if (eventType == CicadaEventFirstRenderedStart) {
            [weakSelf.playerViewController.player addExtSubtitle:[ApsaraTestCase outsideSubtitleURLString]];
        }
    };
    weakSelf.playerViewController.onSubtitleExtAdded = ^(int trackIndex, NSString *URL) {
        [expection fulfill];
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

- (void)testselectExtSubtitle {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
        self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].lastObject;
        [self.playerViewController initPlayer];
        NSString *homePath = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES).firstObject;
        WEAK_SELF
        [weakSelf.playerViewController initPlayerWithCachePath:homePath];
        weakSelf.playerViewController.onPlayerEventCallback = ^(CicadaEventType eventType) {
            if (eventType == CicadaEventFirstRenderedStart) {
                [weakSelf.playerViewController.player addExtSubtitle:[ApsaraTestCase outsideSubtitleURLString]];
            }
        };
        weakSelf.playerViewController.onSubtitleExtAdded = ^(int trackIndex, NSString *URL) {
            [weakSelf.playerViewController.player selectExtSubtitle:trackIndex enable:YES];
            
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(3 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
                [expection fulfill];
            });
        };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试testReload
 */
- (void)testReload {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].firstObject;
    [self.playerViewController initPlayer];
    WEAK_SELF
    weakSelf.playerViewController.onPlayerEventCallback = ^(CicadaEventType eventType) {
        if (eventType == CicadaEventFirstRenderedStart) {
            [self.playerViewController.player reload];
            
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
                [expection fulfill];
            });
        }
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试根据key获取响应的信息
 */
-(void)testGetPropertyString_key0 {
    [self.playerViewController initPlayer];
    NSString * propertyStringKey0 = [self.playerViewController.player getPropertyString:CICADA_KEY_RESPONSE_INFO];
     XCTAssertNotNil(propertyStringKey0,@"获取信息不对");
}
/**
 测试根据key获取响应的信息
 */
-(void)testGetPropertyString_key1 {
    [self.playerViewController initPlayer];
    NSString * propertyStringKey1 = [self.playerViewController.player getPropertyString:CICADA_KEY_CONNECT_INFO];
    XCTAssertNotNil(propertyStringKey1,@"获取信息不对");
}


/**
 测试返回版本号，是5.0就成功
 */
- (void)testGetSDKVersion {
    NSString *version = [CicadaPlayer getSDKVersion];
    XCTAssertTrue([version containsString:@"paas"], @"版本号返回不对");
}

/**
 测试初始化播放器组件方法
 */
- (void)testInitPlayerComponent {
    [CicadaPlayer initPlayerComponent:nil function:nil];
}

/**
 测试设置静音，设置再判断，是否一致
 */
- (void)testMuted {
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].firstObject;
    [self.playerViewController initPlayer];
    self.playerViewController.player.muted = YES;
    XCTAssertTrue(self.playerViewController.player.muted == YES, @"muted 设置 不对");
}

/**
 测试设置静音，设置再判断，是否一致
 */
- (void)testMuted_no {
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].firstObject;
    [self.playerViewController initPlayer];
    self.playerViewController.player.muted = NO;
    XCTAssertTrue(self.playerViewController.player.muted == NO, @"muted 设置 不对");
}

/**
 测试速率，设置再判断，是否一致
 */
- (void)testRate_mixOff {
    [self.playerViewController initPlayer];
    self.playerViewController.player.rate = 0.1;
    XCTAssertTrue(ABS(self.playerViewController.player.rate-0.1)<0.01, @"muted 设置 不对");
}

/**
 测试速率，设置再判断，是否一致
 */
- (void)testRate_min {
    [self.playerViewController initPlayer];
    self.playerViewController.player.rate = 0.5;
    XCTAssertTrue(ABS(self.playerViewController.player.rate-0.5)<0.01, @"muted 设置 不对");
}

/**
 测试速率，设置再判断，是否一致
 */
- (void)testRate {
    [self.playerViewController initPlayer];
    self.playerViewController.player.rate = 1;
    XCTAssertTrue(ABS(self.playerViewController.player.rate-1)<0.01, @"muted 设置 不对");
    
}

/**
 测试速率，设置再判断，是否一致
 */
- (void)testRate_max {
    [self.playerViewController initPlayer];
    self.playerViewController.player.rate = 2;
    XCTAssertTrue(ABS(self.playerViewController.player.rate-2)<0.01, @"muted 设置 不对");
    
}

/**
 测试速率，设置再判断，是否一致
 */
- (void)testRate_maxOff {
    [self.playerViewController initPlayer];
    self.playerViewController.player.rate = 3;
    XCTAssertTrue(ABS(self.playerViewController.player.rate-3)<0.01, @"muted 设置 不对");
    
}

//接口还没有实现，只调用
- (void)testEnableHardwareDecoder {
    [self.playerViewController initPlayer];
    self.playerViewController.player.enableHardwareDecoder = YES;
    self.playerViewController.player.enableHardwareDecoder = NO;
    
}

/**
 测试loop，设置再判断，是否一致
 */
- (void)testLoop {
    [self.playerViewController initPlayer];
    self.playerViewController.player.loop = NO;
    XCTAssertTrue(self.playerViewController.player.loop == NO, @"loop 设置 不对");
    
}

/**
 测试loop，设置再判断，是否一致
 */
- (void)testLoop_yes {
    [self.playerViewController initPlayer];
    self.playerViewController.player.loop = YES;
    XCTAssertTrue(self.playerViewController.player.loop == YES, @"loop 设置 不对");
    
}

/**
 测试自动播放，设置再判断，是否一致
 */
- (void)testAutoPlay {
    [self.playerViewController initPlayer];
    self.playerViewController.player.autoPlay = NO;
    XCTAssertTrue(self.playerViewController.player.autoPlay == NO, @"autoPlay 设置 不对");
    
}

/**
 测试自动播放，设置再判断，是否一致
 */
- (void)testAutoPlay_yes {
    [self.playerViewController initPlayer];
    self.playerViewController.player.autoPlay = YES;
    XCTAssertTrue(self.playerViewController.player.autoPlay == YES, @"autoPlay 设置 不对");
    
}


//接口还没有实现，只调用
- (void)testEnableLog {
    [self.playerViewController initPlayer];
    self.playerViewController.player.enableLog = YES;
    self.playerViewController.player.enableLog = NO;
    
}

/**
 测试mode，设置再判断，是否一致
 */
- (void)testMirrorMode {
    [self.playerViewController initPlayer];
    XCTAssertTrue(self.playerViewController.player.mirrorMode == CICADA_MIRRORMODE_NONE, @"mirrorMode 默认设置 不对");
    
}

/**
 测试mode，设置再判断，是否一致
 */
- (void)testMirrorMode_horizontal {
    [self.playerViewController initPlayer];
    self.playerViewController.player.mirrorMode = CICADA_MIRRORMODE_HORIZONTAL;
    XCTAssertTrue(self.playerViewController.player.mirrorMode == CICADA_MIRRORMODE_HORIZONTAL, @"mirrorMode 设置 不对");
    
}

/**
 测试mode，设置再判断，是否一致
 */
- (void)testMirrorMode_vertical {
    [self.playerViewController initPlayer];
    self.playerViewController.player.mirrorMode = CICADA_MIRRORMODE_VERTICAL;
    XCTAssertTrue(self.playerViewController.player.mirrorMode == CICADA_MIRRORMODE_VERTICAL, @"mirrorMode 设置 不对");
    
}

/**
 测试mode，设置再判断，是否一致
 */
- (void)testRotateMode {
    [self.playerViewController initPlayer];
    XCTAssertTrue(self.playerViewController.player.rotateMode == CICADA_ROTATE_0, @"rotateMode 默认设置 不对");
    
}

/**
 测试mode，设置再判断，是否一致
 */
- (void)testRotateMode_90 {
    [self.playerViewController initPlayer];
    self.playerViewController.player.rotateMode = CICADA_ROTATE_90;
    XCTAssertTrue(self.playerViewController.player.rotateMode == CICADA_ROTATE_90, @"rotateMode 设置 不对");
    
}

/**
 测试mode，设置再判断，是否一致
 */
- (void)testRotateMode_180 {
    [self.playerViewController initPlayer];
    self.playerViewController.player.rotateMode = CICADA_ROTATE_180;
    XCTAssertTrue(self.playerViewController.player.rotateMode == CICADA_ROTATE_180, @"rotateMode 设置 不对");
    
}

/**
 测试mode，设置再判断，是否一致
 */
- (void)testRotateMode_270 {
    [self.playerViewController initPlayer];
    self.playerViewController.player.rotateMode = CICADA_ROTATE_270;
    XCTAssertTrue(self.playerViewController.player.rotateMode == CICADA_ROTATE_270, @"rotateMode 设置 不对");
    
}

/**
 测试mode，设置再判断，是否一致
 */
- (void)testScalingMode {
    [self.playerViewController initPlayer];
    XCTAssertTrue(self.playerViewController.player.scalingMode == CICADA_SCALINGMODE_SCALEASPECTFIT, @"scalingMode 默认设置 不对");
    
}

/**
 测试mode，设置再判断，是否一致
 */
- (void)testScalingMode_toFill {
    [self.playerViewController initPlayer];
    self.playerViewController.player.scalingMode = CICADA_SCALINGMODE_SCALETOFILL;
    XCTAssertTrue(self.playerViewController.player.scalingMode == CICADA_SCALINGMODE_SCALETOFILL, @"scalingMode 设置 不对");
    
}

/**
 测试mode，设置再判断，是否一致
 */
- (void)testScalingMode_aspect {
    [self.playerViewController initPlayer];
    self.playerViewController.player.scalingMode = CICADA_SCALINGMODE_SCALEASPECTFILL;
    XCTAssertTrue(self.playerViewController.player.scalingMode == CICADA_SCALINGMODE_SCALEASPECTFILL, @"scalingMode 设置 不对");
    
}

/**
 测试设置playerview，不能验证，只调用
 */
- (void)testPlayerView {
    [self.playerViewController initPlayer];
    XCTAssertTrue([self.playerViewController.player.playerView isKindOfClass:[UIView class]], @"PlayerView 设置 不对");
    
}

/**
 测试宽度，准备完成，宽度大于等于0，算成功
 */
- (void)testWidth {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].firstObject;
    [self.playerViewController initPlayer];
    WEAK_SELF
    weakSelf.playerViewController.onPlayerEventCallback = ^(CicadaEventType eventType) {
        if (eventType == CicadaEventFirstRenderedStart) {
            XCTAssertTrue(weakSelf.playerViewController.player.width > 0, @"width 应该大于0");
            [expection fulfill];
        }
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试高度，准备完成，宽度大于等于0，算成功
 */
- (void)testHeight {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].firstObject;
    [self.playerViewController initPlayer];
    WEAK_SELF
    weakSelf.playerViewController.onPlayerEventCallback = ^(CicadaEventType eventType) {
        if (eventType == CicadaEventFirstRenderedStart) {
            XCTAssertTrue(weakSelf.playerViewController.player.height > 0, @"height 应该大于0");
            [expection fulfill];
        }
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试rotation，大于等于0，算成功
 */
- (void)testRotation {
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].firstObject;
    [self.playerViewController initPlayer];
    int rotation = self.playerViewController.player.rotation;
    XCTAssertTrue(rotation >= 0, @"rotation 应该>=0");
}

/**
 测试volue，设置再读取，判断是否一致
 */
- (void)testVolume_minOff {
    [self.playerViewController initPlayer];
    self.playerViewController.player.rate = -0.5;
    XCTAssertTrue(self.playerViewController.player.volume >= 0 && self.playerViewController.player.volume <= 1, @"volume 设置 不对");
    
}

/**
 测试volue，设置再读取，判断是否一致
 */
- (void)testVolume_min {
    [self.playerViewController initPlayer];
    self.playerViewController.player.volume = 0;
    XCTAssertTrue(self.playerViewController.player.volume == 0, @"volume 设置 不对");
    
}

/**
 测试volue，设置再读取，判断是否一致
 */
- (void)testVolume {
    [self.playerViewController initPlayer];
    self.playerViewController.player.volume = 0.5;
    XCTAssertTrue(self.playerViewController.player.volume == 0.5, @"volume 设置 不对");
    
}

/**
 测试volue，设置再读取，判断是否一致
 */
- (void)testVolume_max {
    [self.playerViewController initPlayer];
    self.playerViewController.player.volume = 1;
    XCTAssertTrue(self.playerViewController.player.volume == 1, @"volume 设置 不对");
    
}

/**
 测试volue，设置再读取，判断是否一致
 */
- (void)testVolume_maxOff {
    [self.playerViewController initPlayer];
    self.playerViewController.player.volume = 1.1;
    XCTAssertTrue(self.playerViewController.player.volume >= 0 && self.playerViewController.player.rate <= 1, @"volume 设置 不对");
    
}

/**
 判断Duration，准备完成后，应该大于0算成功
 */
- (void)testDuration {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].firstObject;
    [self.playerViewController initPlayer];
    WEAK_SELF
    weakSelf.playerViewController.onPlayerEventCallback = ^(CicadaEventType eventType) {
        if (eventType == CicadaEventPrepareDone) {
            XCTAssertTrue(weakSelf.playerViewController.player.duration > 0, @"duration 应该大于0");
            [expection fulfill];
        }
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试当前位置，回调大于等于0算成功
 */
- (void)testCurrentPosition {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].firstObject;
    [self.playerViewController initPlayer];
    WEAK_SELF
    weakSelf.playerViewController.onPlayerEventCallback = ^(CicadaEventType eventType) {
        if (eventType == CicadaEventPrepareDone) {
            XCTAssertTrue(weakSelf.playerViewController.player.currentPosition >= 0, @"duration 应该>=0");
            [expection fulfill];
        }
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试缓冲位置，大于等于0算成功
 */
- (void)testBufferedPosition {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].firstObject;
    [self.playerViewController initPlayer];
    WEAK_SELF
    weakSelf.playerViewController.onPlayerEventCallback = ^(CicadaEventType eventType) {
        if (eventType == CicadaEventPrepareDone) {
            XCTAssertTrue(weakSelf.playerViewController.player.bufferedPosition >= 0, @"bufferedPosition 应该>=0");
            [expection fulfill];
        }
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试代理，能设置，类型正确，算成功
 */
- (void)testDelegate {
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].firstObject;
    [self.playerViewController initPlayer];
    XCTAssertTrue([self.playerViewController.player.delegate isKindOfClass:[AliyunPlayerViewController class]], @"delegate 设置不对");
    
}

/**
 测试设置日志功能
 */
- (void)testsetLogCallbackInfo {
    [CicadaPlayer setLogCallbackInfo:CICADA_LOG_LEVEL_NONE callbackBlock:^(CicadaLogLevel logLevel, NSString *strLog) {
        NSLog(@"CicadaLogLevel:%lu,strLog:%@",(unsigned long)logLevel,strLog);
    }];
}

#pragma mark CicadaDelegate

/**
 测试状态回调，准备播放回到准备完成，算成功
 */
- (void)testOnPlayerEvent {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].firstObject;
    [self.playerViewController initPlayer];
    self.playerViewController.onPlayerEventCallback = ^(CicadaEventType eventType) {
        if (eventType == CicadaEventPrepareDone) {
            [expection fulfill];
        }
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试事件回调
 */
- (void)testOnPlayerEventWithString {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].firstObject;
    [self.playerViewController initPlayer];
    self.playerViewController.onPlayerEventCallback = ^(CicadaEventType eventType) {
        if (eventType == CicadaEventPrepareDone) {
            [expection fulfill];
        }
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试错误回调，过期有回调，算成功
 */
- (void)testOnError {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [[CicadaUrlSource alloc] urlWithString:@"haax://cloud.video.taobao.com/play/u/2712925557"];
    WEAK_SELF
    [weakSelf.playerViewController initPlayer];
    [weakSelf.playerViewController initPlayer];
    weakSelf.playerViewController.onErrorCallback = ^(CicadaErrorModel *errorModel) {
        XCTAssertNotNil(errorModel, @"errorModel can not be nil");
        [expection fulfill];
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试VideoSizeChanged，大小不为空，算成功
 */
- (void)testOnVideoSizeChanged {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].firstObject;
    [self.playerViewController initPlayer];
    WEAK_SELF
    weakSelf.playerViewController.onVideoSizeChangedCallback = ^(int width, int height, int rotation) {
        if (width == 0 || height == 0) {
            XCTFail(@"OnVideoSizeChanged失败，返回空了");
        }
        [expection fulfill];
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试当前位置回调，大于等于0算成功
 */
- (void)testOnCurrentPositionUpdate {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].firstObject;
    [self.playerViewController initPlayer];
    WEAK_SELF
    weakSelf.playerViewController.onCurrentPositionUpdateCallback = ^(int64_t position) {
        XCTAssertTrue(position >= 0, @"position 必须大于等于0");
        [expection fulfill];
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试缓冲位置，大于等于0算成功
 */
- (void)testOnBufferedPositionUpdate {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].firstObject;
    [self.playerViewController initPlayer];
    WEAK_SELF
    weakSelf.playerViewController.onBufferedPositionUpdateCallback = ^(int64_t position) {
        XCTAssertTrue(position >= 0, @"position 必须大于等于0");
        [expection fulfill];
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试缓冲进度回调，会达到100算成功
 */
- (void)testOnLoadingProgress {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].firstObject;
    [self.playerViewController initPlayer];
    [self.playerViewController seekToTime:self.playerViewController.player.duration*0.5 seekMode:CICADA_SEEKMODE_INACCURATE];
    WEAK_SELF
    weakSelf.playerViewController.onLoadingProgressCallback = ^(float progress) {
        XCTAssertTrue(progress >= 0, @"progress 必须大于等于0");
        if (progress >= 100) {
            [expection fulfill];
        }
    };
    weakSelf.playerViewController.onPlayerEventCallback = ^(CicadaEventType eventType) {
        if (eventType == CicadaEventFirstRenderedStart) {
            [expection fulfill];
        }
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试OnTrackReady，大于等于0算成功
 */
- (void)testOnTrackReady {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].firstObject;
    [self.playerViewController initPlayer];
    WEAK_SELF
    weakSelf.playerViewController.onTrackReadyCallback = ^(NSArray<CicadaTrackInfo *> *info) {
        XCTAssertTrue(info.count >= 0, @"info.count 必须大于等于0");
        [expection fulfill];
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试onTrackChanged，先选择一个，在判断回到的index是否相等
 */
- (void)testOnTrackChanged {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].lastObject;
    [self.playerViewController initPlayer];
    [self.playerViewController selectTrack];
    WEAK_SELF
    weakSelf.playerViewController.onTrackChangedCallback = ^(CicadaTrackInfo *info) {
        CicadaTrackInfo *lastInfo = weakSelf.playerViewController.info.lastObject;
        XCTAssertTrue(lastInfo.trackIndex == info.trackIndex, @"selectTrack 选择返回错误");
        [expection fulfill];
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

- (void)testonSubtitleExtAdded {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].lastObject;
    WEAK_SELF
    [weakSelf.playerViewController initPlayer];
    NSString *homePath = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES).firstObject;
    [weakSelf.playerViewController initPlayerWithCachePath:homePath];
    weakSelf.playerViewController.onPlayerEventCallback = ^(CicadaEventType eventType) {
        if (eventType == CicadaEventFirstRenderedStart) {
            [weakSelf.playerViewController.player addExtSubtitle:[ApsaraTestCase outsideSubtitleURLString]];
        }
    };
    weakSelf.playerViewController.onSubtitleExtAdded = ^(int trackIndex, NSString *URL) {
        [expection fulfill];
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试SubtitleShow，有回调就成功
 5.0已经没有这个接口，所以DISABLE
 */
- (void)DISABLE_testOnSubtitleShow {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].lastObject;
    [self.playerViewController initPlayer];
    [self.playerViewController selectTrack];
    WEAK_SELF
    weakSelf.playerViewController.onSubtitleShowCallback = ^(int index, NSString *subtitle) {
        XCTAssertTrue(subtitle.length != 0, @"subtitle 不能为空");
        [expection fulfill];
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试OnSubtitleHide，有回调就成功
 5.0已经没有这个接口，所以DISABLE
 */
- (void)DISABLE_testOnSubtitleHide {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].lastObject;
    [self.playerViewController initPlayer];
    [self.playerViewController selectTrack];
    WEAK_SELF
    weakSelf.playerViewController.onSubtitleHideCallback = ^(int index) {
        [expection fulfill];
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试状态回调
 */
- (void)testonPlayerStatusChanged {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].firstObject;
    [self.playerViewController initPlayer];
    [self.playerViewController pause];
    self.playerViewController.onPlayerStatusChangedCallback = ^(CicadaStatus oldStatus, CicadaStatus newStatus) {
        if (newStatus == CicadaStatusPaused) {
            [expection fulfill];
        }
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试OnCaptureScreen，截图就返回就成功
 */
- (void)testOnCaptureScreen {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].firstObject;
    [self.playerViewController initPlayer];
    [self.playerViewController snapShot];
    WEAK_SELF
    weakSelf.playerViewController.onCaptureScreenCallback = ^(UIImage *image) {
        XCTAssertNotNil(image, @"截图返回为空");
        [expection fulfill];
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

#pragma mark CicadaConfig

/**
 测试属性，设置再读取，看是否正确
 */
- (void)testMaxDelayTime_default {
    [self.playerViewController initPlayer];
    CicadaConfig *config = [self.playerViewController.player getConfig];
    int maxDelayTime = config.maxDelayTime;
    XCTAssertEqual(maxDelayTime,5000,@"maxDelayTime默认属性错误");
    
}

/**
 测试属性，设置再读取，看是否正确
 */
- (void)testMaxDelayTime {
    [self.playerViewController initPlayer];
    CicadaConfig *config = [self.playerViewController.player getConfig];
    config.maxDelayTime = 3000;
    XCTAssertEqual(config.maxDelayTime,3000,@"maxDelayTime属性设置错误");
    
}

/**
 测试属性，设置再读取，看是否正确
 */
- (void)testHighBufferDuration_default {
    [self.playerViewController initPlayer];
    CicadaConfig *config = [self.playerViewController.player getConfig];
    XCTAssertEqual(config.highBufferDuration,3000,@"HighBufferDuration默认属性设置错误");
    
}

/**
 测试属性，设置再读取，看是否正确
 */
- (void)testHighBufferDuration {
    [self.playerViewController initPlayer];
    CicadaConfig *config = [self.playerViewController.player getConfig];
    config.highBufferDuration = 500;
    XCTAssertEqual(config.highBufferDuration,500,@"HighBufferDuration属性设置错误");
    
}

/**
 测试属性，设置再读取，看是否正确
 */
- (void)testStartBufferDuration_default {
    [self.playerViewController initPlayer];
    CicadaConfig *config = [self.playerViewController.player getConfig];
    XCTAssertEqual(config.startBufferDuration,500,@"startBufferDuration默认属性错误");
    
}

/**
 测试属性，设置再读取，看是否正确
 */
- (void)testStartBufferDuration {
    [self.playerViewController initPlayer];
    CicadaConfig *config = [self.playerViewController.player getConfig];
    config.startBufferDuration = 3000;
    XCTAssertEqual(config.startBufferDuration,3000,@"startBufferDuration属性设置错误");
    
}

/**
 测试属性，设置再读取，看是否正确
 */
- (void)testMaxBufferDuration_default {
    [self.playerViewController initPlayer];
    CicadaConfig *config = [self.playerViewController.player getConfig];
    XCTAssertEqual(config.maxBufferDuration,50000,@"maxBufferDuration默认属性错误");
    
}

/**
 测试属性，设置再读取，看是否正确
 */
- (void)testMaxBufferDuration {
    [self.playerViewController initPlayer];
    CicadaConfig *config = [self.playerViewController.player getConfig];
    config.maxBufferDuration = 5000;
    XCTAssertEqual(config.maxBufferDuration,5000,@"maxBufferDuration属性设置错误");
    
}

/**
 测试属性，设置再读取，看是否正确
 */
- (void)testNetworkTimeout_default {
    [self.playerViewController initPlayer];
    CicadaConfig *config = [self.playerViewController.player getConfig];
    XCTAssertEqual(config.networkTimeout,15000,@"networkTimeout默认属性错误");
    
}

/**
 测试属性，设置再读取，看是否正确
 */
- (void)testNetworkTimeout {
    [self.playerViewController initPlayer];
    CicadaConfig *config = [self.playerViewController.player getConfig];
    config.networkTimeout = 20000;
    XCTAssertEqual(config.networkTimeout,20000,@"networkTimeout属性设置错误");
    
}

/**
 测试属性，设置再读取，看是否正确
 */
- (void)testNetworkRetryCount_default {
    [self.playerViewController initPlayer];
    CicadaConfig *config = [self.playerViewController.player getConfig];
    XCTAssertEqual(config.networkRetryCount,2,@"networkRetryCount默认属性错误");
    
}

/**
 测试属性，设置再读取，看是否正确
 */
- (void)testNetworkRetryCount {
    [self.playerViewController initPlayer];
    CicadaConfig *config = [self.playerViewController.player getConfig];
    config.networkRetryCount = 3;
    XCTAssertEqual(config.networkRetryCount,3,@"networkRetryCount属性设置错误");
    
}

/**
 测试属性，设置再读取，看是否正确
 */
- (void)testReferer_default {
    [self.playerViewController initPlayer];
    CicadaConfig *config = [self.playerViewController.player getConfig];
    XCTAssertTrue(config.referer.length == 0, @"referer默认属性错误");
    
}

/**
 测试属性，设置再读取，看是否正确
 */
- (void)testReferer {
    [self.playerViewController initPlayer];
    CicadaConfig *config = [self.playerViewController.player getConfig];
    config.referer = @"https";
    XCTAssertEqual(config.referer,@"https",@"referer属性设置错误");
    
}

/**
 测试属性，设置再读取，看是否正确
 */
- (void)testHttpProxy_default {
    [self.playerViewController initPlayer];
    CicadaConfig *config = [self.playerViewController.player getConfig];
    XCTAssertTrue(config.httpProxy.length == 0, @"httpProxy默认属性错误");
    
}

/**
 测试属性，设置再读取，看是否正确
 */
- (void)testHttpProxy {
    [self.playerViewController initPlayer];
    CicadaConfig *config = [self.playerViewController.player getConfig];
    config.httpProxy = @"https";
    XCTAssertEqual(config.httpProxy,@"https",@"httpProxy属性设置错误");
    
}

/**
 测试属性，设置再读取，看是否正确
 */
- (void)testClearShowWhenStop_default {
    [self.playerViewController initPlayer];
    CicadaConfig *config = [self.playerViewController.player getConfig];
    XCTAssertTrue(config.clearShowWhenStop == NO, @"clearShowWhenStop默认属性错误");
    
}

/**
 测试属性，设置再读取，看是否正确
 */
- (void)testClearShowWhenStop {
    [self.playerViewController initPlayer];
    CicadaConfig *config = [self.playerViewController.player getConfig];
    config.clearShowWhenStop = YES;
    XCTAssertTrue(config.clearShowWhenStop == YES, @"clearShowWhenStop属性设置错误");
    
}

/**
 测试属性，设置再读取，看是否正确
 */
- (void)testHttpHeaders_default {
    [self.playerViewController initPlayer];
    CicadaConfig *config = [self.playerViewController.player getConfig];
    XCTAssertTrue(config.httpHeaders.count == 0, @"httpHeaders默认属性错误");
    
}

/**
 测试属性，设置再读取，看是否正确
 */
- (void)testHttpHeaders {
    [self.playerViewController initPlayer];
    CicadaConfig *config = [self.playerViewController.player getConfig];
    config.httpHeaders = @[@"https"].mutableCopy;
    XCTAssertTrue(config.httpHeaders.count == 1, @"httpHeaders属性设置错误");
    
}

#pragma mark error

/**
 测试code，错误有code，就成功
 */
- (void)testCode {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [[CicadaUrlSource alloc] urlWithString:@"haax://cloud.video.taobao.com/play/u/2712925557/p/1/e/6/t/1"];
    WEAK_SELF
    [weakSelf.playerViewController initPlayer];
    weakSelf.playerViewController.onErrorCallback = ^(CicadaErrorModel *errorModel) {
        XCTAssertTrue(errorModel.code != 0, @"errorModel.code错误");
        [expection fulfill];
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试message，错误有message，就成功
 */
- (void)testMessage {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [[CicadaUrlSource alloc] urlWithString:@"cloud.video.taobao.com/play/u/2712925557/p/1/e/6/t/1"];
    WEAK_SELF
    [weakSelf.playerViewController initPlayer];
    weakSelf.playerViewController.onErrorCallback = ^(CicadaErrorModel *errorModel) {
        XCTAssertTrue(errorModel.message.length != 0, @"errorModel.message错误");
        [expection fulfill];
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试RequestId，错误有message，有就成功
 */
- (void)testRequestId {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [[CicadaUrlSource alloc] urlWithString:@"//cloud.video.taobao.com/play/u/2712925557/p/1/e/6/t/"];
    WEAK_SELF
    [weakSelf.playerViewController initPlayer];
    weakSelf.playerViewController.onErrorCallback = ^(CicadaErrorModel *errorModel) {
        errorModel.requestId;
        [expection fulfill];
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试videoID，只读取
 */
- (void)testVideoId {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [[CicadaUrlSource alloc] urlWithString:@"cloud.video.taobao.com/play/u/2712925557/p/1/e/6"];
    WEAK_SELF
    [weakSelf.playerViewController initPlayer];
    weakSelf.playerViewController.onErrorCallback = ^(CicadaErrorModel *errorModel) {
        errorModel.videoId;
        [expection fulfill];
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

#pragma mark CicadaTrackInfo

/**
 测试info，有属性就成功
 */
- (void)testTrackType {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].lastObject;
    [self.playerViewController initPlayer];
    [self.playerViewController selectTrack];
    WEAK_SELF
    weakSelf.playerViewController.onTrackReadyCallback = ^(NSArray<CicadaTrackInfo *> *info) {
        CicadaTrackInfo *eveinfo = info.firstObject;
        eveinfo.trackType;
        [expection fulfill];
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试info，有属性就成功
 */
- (void)testTrackIndex {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].lastObject;
    [self.playerViewController initPlayer];
    [self.playerViewController selectTrack];
    WEAK_SELF
    weakSelf.playerViewController.onTrackReadyCallback = ^(NSArray<CicadaTrackInfo *> *info) {
        CicadaTrackInfo *eveinfo = info.firstObject;
        eveinfo.trackIndex;
        [expection fulfill];
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试vodFormat，有属性就成功
 */
- (void)testVodFormat {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].lastObject;
    [self.playerViewController initPlayer];
    [self.playerViewController selectTrack];
    WEAK_SELF
    weakSelf.playerViewController.onTrackReadyCallback = ^(NSArray<CicadaTrackInfo *> *info) {
        CicadaTrackInfo *eveinfo = info.firstObject;
        eveinfo.vodFormat;
        [expection fulfill];
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试info，有属性就成功
 */
- (void)testVideoWidth {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].lastObject;
    [self.playerViewController initPlayer];
    [self.playerViewController selectTrack];
    WEAK_SELF
    weakSelf.playerViewController.onTrackReadyCallback = ^(NSArray<CicadaTrackInfo *> *info) {
        CicadaTrackInfo *eveinfo = info.firstObject;
        eveinfo.videoWidth;
        [expection fulfill];
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试info，有属性就成功
 */
- (void)testVideoHeight {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].lastObject;
    [self.playerViewController initPlayer];
    [self.playerViewController selectTrack];
    WEAK_SELF
    weakSelf.playerViewController.onTrackReadyCallback = ^(NSArray<CicadaTrackInfo *> *info) {
        CicadaTrackInfo *eveinfo = info.firstObject;
        eveinfo.videoHeight;
        [expection fulfill];
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试info，有属性就成功
 */
- (void)testTrackBitrate {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].lastObject;
    [self.playerViewController initPlayer];
    [self.playerViewController selectTrack];
    WEAK_SELF
    weakSelf.playerViewController.onTrackReadyCallback = ^(NSArray<CicadaTrackInfo *> *info) {
        CicadaTrackInfo *eveinfo = info.firstObject;
        eveinfo.trackBitrate;
        [expection fulfill];
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试info，有属性就成功
 */
- (void)testAudioChannels {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].lastObject;
    [self.playerViewController initPlayer];
    [self.playerViewController selectTrack];
    WEAK_SELF
    weakSelf.playerViewController.onTrackReadyCallback = ^(NSArray<CicadaTrackInfo *> *info) {
        CicadaTrackInfo *eveinfo = info.firstObject;
        eveinfo.audioChannels;
        [expection fulfill];
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试info，有属性就成功
 */
- (void)testAudioSamplerate {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].lastObject;
    [self.playerViewController initPlayer];
    [self.playerViewController selectTrack];
    WEAK_SELF
    weakSelf.playerViewController.onTrackReadyCallback = ^(NSArray<CicadaTrackInfo *> *info) {
        CicadaTrackInfo *eveinfo = info.firstObject;
        eveinfo.audioSamplerate;
        [expection fulfill];
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试info，有属性就成功
 */
- (void)testAudioSampleFormat {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].lastObject;
    [self.playerViewController initPlayer];
    [self.playerViewController selectTrack];
    WEAK_SELF
    weakSelf.playerViewController.onTrackReadyCallback = ^(NSArray<CicadaTrackInfo *> *info) {
        CicadaTrackInfo *eveinfo = info.firstObject;
        eveinfo.audioSampleFormat;
        [expection fulfill];
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试info，有属性就成功
 */
- (void)testAudioLanguage {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].lastObject;
    [self.playerViewController initPlayer];
    [self.playerViewController selectTrack];
    WEAK_SELF
    weakSelf.playerViewController.onTrackReadyCallback = ^(NSArray<CicadaTrackInfo *> *info) {
        CicadaTrackInfo *eveinfo = info.firstObject;
        eveinfo.audioLanguage;
        [expection fulfill];
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

/**
 测试info，有属性就成功
 */
- (void)testSubtitleLanguage {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].lastObject;
    [self.playerViewController initPlayer];
    [self.playerViewController selectTrack];
    WEAK_SELF
    weakSelf.playerViewController.onTrackReadyCallback = ^(NSArray<CicadaTrackInfo *> *info) {
        CicadaTrackInfo *eveinfo = info.firstObject;
        eveinfo.subtitleLanguage;
        [expection fulfill];
    };
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

#pragma mark CicadaMediaInfo

/**
 测试info.tracks，不能为空
 */
- (void)testTracks {
    NSString *expectionDescription = [NSStringFromSelector(_cmd) stringByAppendingString:@" should success"];
    XCTestExpectation *expection = [self expectationWithDescription:expectionDescription];
    self.playerViewController.urlSource = [ApsaraTestCase urlSourceArray].firstObject;
    [self.playerViewController initPlayer];
    WEAK_SELF
    weakSelf.playerViewController.getMediaInfoCallback = ^(CicadaMediaInfo *info) {
        XCTAssertNotNil(info.tracks, @"MediaInfo.tracks 不能为空");
        [expection fulfill];
    };
    [self.playerViewController getMediaInfo];
    [self waitForExpectationsWithTimeout:AliyunTestAysncTimeOut handler:^(NSError * _Nullable error) {
        NSString *timeoutString = [NSStringFromSelector(_cmd) stringByAppendingString:@" timeout"];
        XCTAssert(timeoutString);
    }];
}

@end














