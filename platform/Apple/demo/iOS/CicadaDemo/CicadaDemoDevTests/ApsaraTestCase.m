//
//  ApsaraTestCase.m
//  AliPlayerAutoTestTests
//
//  Created by 郦立 on 2019/1/17.
//  Copyright © 2019年 wb-qxx397776. All rights reserved.
//

#import "ApsaraTestCase.h"

@implementation ApsaraTestCase

+ (NSArray <CicadaUrlSource *>*)urlSourceArray {
    CicadaUrlSource *source = [[CicadaUrlSource alloc] urlWithString:@"http://cloud.video.taobao.com/play/u/2712925557/p/1/e/6/t/1/40050769.mp4"];
    CicadaUrlSource *source1 = [[CicadaUrlSource alloc] urlWithString:@"http://livetest.aliyunlive.com/5dbabcec4df04d20b2707369c1598aaa/7613472a0dd34731b1a1457ed5469b3f.m3u8?auth_key=1593694564-0-0-e6af5fdea33414eb065c290e1979d87d"];
    return @[source,source1];
}

+ (NSArray *)uidSTSArray {
    return @[@"2eb0bda763c84808890a3178f1ee7acd",@"2553de466ebd44d0ae03ea501befa332",@"606a1932980c4659b0cea0c002c5f4a2",@"99196bffe62c45c3af60234fd5c366a0",@"edc82fc54b2a495e97d70930954a4e6d"];
}

+ (NSArray *)uidURLArray {
    return @[@"https://alivc-demo-vod.aliyuncs.com/588dd53d0519401696360614da433d9b/ebfafff4f1af49f894a0228aa447caed-0f5b3ccd69b6df26e383805ceca401de-ld.mp4",@"https://alivc-demo-vod.aliyuncs.com/3685cd1f627a4375a285fd67e21852b5/d9d49cf93c7248639e6837329b3398e4-01aee5939726691d84bba4e73466c061-ld.mp4",@"https://alivc-demo-vod.aliyuncs.com/9360a4e606a54db0b3e11b0a7dfb0ab2/8020aea99159425f9d7fe90fbf600372-3966fdfd045bd3a0742d59835bc31aa5-fd.mp4",@"https://alivc-demo-vod.aliyuncs.com/9360a4e606a54db0b3e11b0a7dfb0ab2/8020aea99159425f9d7fe90fbf600372-b31eb249323ba9ee8ffe6da3d16e4c03-sd.mp4"];
}

+ (NSArray <NSUUID *>*)uuidArray {
    return @[[NSUUID UUID],[NSUUID UUID],[NSUUID UUID],[NSUUID UUID],[NSUUID UUID]];
}

+ (NSArray *)downloaderVidArray {
    return @[@"6609a2f737cb43e1a79ec2bc6aee781b",@"7b1e7efe890147f8a7008e5d5cac98a9",@"969d8b3690b04adbbfda98077a34c32b"];
}

+ (NSString *)timeShiftUrlString {
    return @"http://qt1.alivecdn.com/timeline/testshift.m3u8?auth_key=1594730859-0-0-b71fd57c57a62a3c2b014f24ca2b9da3";
}

+ (NSString *)timeShiftUrlCurrentString {
    NSTimeInterval currentSeconds = [[NSDate date] timeIntervalSince1970]; //秒
    NSString *currentLive = [NSString stringWithFormat:@"http://qt1.alivecdn.com/openapi/timeline/query?auth_key=1594731135-0-0-61c9bd253b29ef4c8017ce05c0953083&app=timeline&stream=testshift&format=ts&lhs_start_unix_s_0=%.0f&lhs_end_unix_s_0=%.0f",(currentSeconds - 5 * 60), (currentSeconds + 5 * 60)];
    return currentLive;
}

+ (NSString *)thumbnailVID {
    return @"63566edb9f61417bb46b0bb2b26cb29e";
}

+ (NSString *)outsideSubtitleURLString {
    return @"http://livetest.aliyunlive.com/5dbabcec4df04d20b2707369c1598aaa/extract/subtitle/en.vtt?auth_key=1593694564-0-0-34653b821bb7c8641456833660bc2d74";
}

@end









