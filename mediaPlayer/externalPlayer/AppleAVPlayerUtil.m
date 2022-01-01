//
//  AppleAVPlayerUtil.m
//  CicadaPlayerSDK
//
//  Created by zhou on 2020/7/26.
//

#import "AppleAVPlayerUtil.h"

@implementation AppleAVPlayerUtil

+ (int64_t)getBufferPosition:(AVPlayerItem *)playerItem {
    NSArray *loadedTimeRanges = playerItem.loadedTimeRanges;
    CMTimeRange timeRange = [loadedTimeRanges.firstObject CMTimeRangeValue];
    NSTimeInterval loadStartSeconds = CMTimeGetSeconds(timeRange.start);
    NSTimeInterval loadDurationSeconds = CMTimeGetSeconds(timeRange.duration);
    NSTimeInterval currentLoadTotalTime = loadStartSeconds + loadDurationSeconds;
    if (isnan(currentLoadTotalTime)) {
        return 0;
    }
    int64_t position = (int64_t)(currentLoadTotalTime * 1000);
    return position;
}

@end
