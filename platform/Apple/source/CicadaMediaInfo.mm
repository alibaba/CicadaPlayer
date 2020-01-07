//
//  CicadaMediaInfo.mm
//  CicadaPlayerSDK
//
//  Created by shiping.csp on 2018/11/16.
//  Copyright © 2018年 com.alibaba.AliyunPlayer. All rights reserved.
//


#import "CicadaMediaInfo.h"


@interface CicadaTrackInfo ()
@end

@implementation CicadaTrackInfo

@synthesize trackType;
@synthesize trackIndex;
@synthesize description;
@synthesize videoWidth;
@synthesize videoHeight;
@synthesize audioChannels;
@synthesize audioSamplerate;
@synthesize audioSampleFormat;
@synthesize audioLanguage;
@synthesize subtitleLanguage;
@synthesize trackBitrate;
@synthesize vodFormat;

- (instancetype)init{
    if (self = [super init]) {
        videoWidth = 0;
        videoHeight = 0;
        audioChannels = 0;
        audioSampleFormat = 0;
        audioSamplerate = 0;
        audioLanguage = @"";
        subtitleLanguage = @"";
        trackBitrate = 0;
        description = @"";
        trackType = CICADA_TRACK_VIDEO;
        trackIndex = 0;
        vodFormat = @"";
    }
    return self;
}

@end


@implementation CicadaThumbnailInfo
@synthesize URL;
- (instancetype)init {
    if (self = [super init]) {
        self.URL = @"";
    }
    return self;
}
@end

@interface CicadaMediaInfo ()
@end

@implementation CicadaMediaInfo
@synthesize tracks;

- (instancetype)init{
    if (self = [super init]) {
        tracks = [[NSMutableArray alloc] init];
    }
    return self;
}

@end
