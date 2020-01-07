//
//  SourceAndPropertyTest.m
//  AliPlayerAutoTestTests
//
//  Created by 郦立 on 2019/1/17.
//  Copyright © 2019年 wb-qxx397776. All rights reserved.
//

#import <XCTest/XCTest.h>
#import "ApsaraTestCase.h"

@interface SourceAndPropertyTest : XCTestCase

@end

@implementation SourceAndPropertyTest

- (void)setUp {
    [super setUp];
    
}

- (void)tearDown {
    [super tearDown];
}

#pragma mark CicadaSource

/**
 测试属性，看读取是否正确
 */
- (void)testCicadaSourceCoverURL {
    CicadaSource *source = [[CicadaSource alloc]init];
    source.coverURL = @"coverURL";
    XCTAssertTrue([source.coverURL isEqualToString:@"coverURL"], @"coverURL 属性设置不对");
}

//接口只读
- (void)testCicadaSourceTitle {
    CicadaSource *source = [[CicadaSource alloc]init];
    NSString *title = source.title;
    NSLog(@"%@",title);
}

//接口只读
- (void)testCicadaSourceQuality {
    CicadaSource *source = [[CicadaSource alloc]init];
    NSString *quality = source.quality;
    NSLog(@"%@",quality);
}

- (void)testCicadaSourceForceQuality {
    CicadaSource *source = [[CicadaSource alloc]init];
    source.forceQuality = YES;
    XCTAssertTrue(source.forceQuality == YES, @"forceQuality 属性设置不对");
    source.forceQuality = NO;
    XCTAssertTrue(source.forceQuality == NO, @"forceQuality 属性设置不对");
}

#pragma mark CicadaUrlSource

/**
 测试属性，看读取是否正确
 */
- (void)testCicadaUrlSource_urlWithString {
    CicadaUrlSource *source = [ApsaraTestCase urlSourceArray].firstObject;
    XCTAssertTrue([source isKindOfClass:[CicadaUrlSource class]], @"CicadaUrlSource urlWithString 创建类型不对");
}

/**
 测试属性，看读取是否正确
 */
- (void)testCicadaUrlSource_fileURLWithPath {
    CicadaUrlSource *source = [[CicadaUrlSource alloc] fileURLWithPath:@"path"];
    XCTAssertTrue([source isKindOfClass:[CicadaUrlSource class]], @"CicadaUrlSource fileURLWithPath 创建类型不对");
}

/**
 测试属性，看读取是否正确
 */
- (void)testCicadaUrlSource_urlWithString_playerUrl {
    CicadaUrlSource *source = [ApsaraTestCase urlSourceArray].firstObject;
    XCTAssertTrue([source.playerUrl.absoluteString isEqualToString:@"http://cloud.video.taobao.com/play/u/2712925557/p/1/e/6/t/1/40050769.mp4"], @"CicadaUrlSource playerUrl 属性初始化出错");
}

/**
 测试属性，看读取是否正确
 */
- (void)testCicadaUrlSource_fileURL_playerUrl {
    CicadaUrlSource *source = [[CicadaUrlSource alloc] fileURLWithPath:@"path"];
    XCTAssertTrue([source.playerUrl.absoluteString containsString:@"path"], @"CicadaUrlSource playerUrl 属性初始化出错");
}

/**
 测试属性，看读取是否正确
 */
- (void)testCicadaUrlSource_set_playerUrl {
    CicadaUrlSource *source = [[CicadaUrlSource alloc] init];
    source.playerUrl = [NSURL URLWithString:@"path"];
    XCTAssertTrue([source.playerUrl.absoluteString containsString:@"path"], @"CicadaUrlSource playerUrl 属性设置出错");
}

- (void)testCicadaUrlSourceCacheFile {
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
    NSString *cachesDir = [paths objectAtIndex:0];
    CicadaUrlSource *source = [ApsaraTestCase urlSourceArray].firstObject;
    source.cacheFile = cachesDir;
    NSString * filePath = source.cacheFile;
     XCTAssertTrue([filePath isEqualToString:source.cacheFile], @"CicadaUrlSource CacheFile 属性出错");
}

#pragma mark CicadaCacheConfig

/**
 测试属性，看读取是否正确
 */
- (void)testCicadaCacheConfig_path_default {
    CicadaCacheConfig *config = [[CicadaCacheConfig alloc]init];
    XCTAssertTrue([config.path containsString:NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES).firstObject], @"CicadaCacheConfig path 默认属性出错");
}

/**
 测试属性，看读取是否正确
 */
- (void)testCicadaCacheConfig_path {
    CicadaCacheConfig *config = [[CicadaCacheConfig alloc]init];
    config.path = @"xxx";
    XCTAssertTrue([config.path isEqualToString:@"xxx"], @"CicadaCacheConfig path 属性设置出错");
}

/**
 测试属性，看读取是否正确
 */
- (void)testCicadaCacheConfig_maxDuration_default {
    CicadaCacheConfig *config = [[CicadaCacheConfig alloc]init];
    XCTAssertTrue(config.maxDuration == 0, @"CicadaCacheConfig maxDuration 默认属性出错");
}

/**
 测试属性，看读取是否正确
 */
- (void)testCicadaCacheConfig_maxDuration {
    CicadaCacheConfig *config = [[CicadaCacheConfig alloc]init];
    config.maxDuration = 5;
    XCTAssertTrue(config.maxDuration == 5, @"CicadaCacheConfig maxDuration 属性设置出错");
}

/**
 测试属性，看读取是否正确
 */
- (void)testCicadaCacheConfig_maxSizeMB_default {
    CicadaCacheConfig *config = [[CicadaCacheConfig alloc]init];
    XCTAssertTrue(config.maxSizeMB == 0, @"CicadaCacheConfig maxSizeMB 默认属性出错");
}

/**
 测试属性，看读取是否正确
 */
- (void)testCicadaCacheConfig_maxSizeMB {
    CicadaCacheConfig *config = [[CicadaCacheConfig alloc]init];
    config.maxSizeMB = 5;
    XCTAssertTrue(config.maxSizeMB == 5, @"CicadaCacheConfig maxSizeMB 属性设置出错");
}

/**
 测试属性，看读取是否正确
 */
- (void)testCicadaCacheConfig_enable_default {
    CicadaCacheConfig *config = [[CicadaCacheConfig alloc]init];
    XCTAssertTrue(config.enable == 0, @"CicadaCacheConfig enable 默认属性出错");
}

/**
 测试属性，看读取是否正确
 */
- (void)testCicadaCacheConfig_enable {
    CicadaCacheConfig *config = [[CicadaCacheConfig alloc]init];
    config.enable = YES;
    XCTAssertTrue(config.enable == YES, @"CicadaCacheConfig enable 属性设置出错");
}

#pragma mark CicadaThumbnailInfo

/**
 测试属性，看读取是否正确
 */
- (void)testCicadaThumbnailInfo_URL_default {
    CicadaThumbnailInfo *info = [[CicadaThumbnailInfo alloc]init];
    XCTAssertTrue(info.URL.length == 0, @"CicadaCacheConfig URL 默认属性出错");
}

/**
 测试属性，看读取是否正确
 */
- (void)testCicadaThumbnailInfo_URL {
    CicadaThumbnailInfo *info = [[CicadaThumbnailInfo alloc]init];
    info.URL = @"url";
    XCTAssertTrue([info.URL isEqualToString:@"url"], @"CicadaCacheConfig URL 属性设置出错");
}

@end












