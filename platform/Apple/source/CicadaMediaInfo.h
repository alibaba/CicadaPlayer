//
//  CicadaMediaInfo.h
//  CicadaPlayerSDK
//
//  Created by shiping.csp on 2018/11/16.
//  Copyright © 2018 com.alibaba.AliyunPlayer. All rights reserved.
//

#ifndef CicadaMediaInfo_h
#define CicadaMediaInfo_h

#import <Foundation/Foundation.h>

const static int SELECT_CICADA_TRACK_VIDEO_AUTO = -1;

/**
 @brief track type
 * CICADA_TRACK_VIDEO: video track
 * CICADA_TRACK_AUDIO: audio track
 * CICADA_TRACK_SUBTITLE: subtitle track
 * CICADA_TRACK_MIXED: have auido and video info in this track
 */
typedef enum CicadaTrackType: NSUInteger {
    CICADA_TRACK_VIDEO,
    CICADA_TRACK_AUDIO,
    CICADA_TRACK_SUBTITLE,
} CicadaTrackType;

typedef enum CicadaVideoHDRType : NSUInteger {
    CICADA_VideoHDRType_SDR,
    CICADA_VideoHDRType_HDR10,
} CicadaVideoHDRType;

OBJC_EXPORT
@interface CicadaTrackInfo : NSObject

/**
 @brief track type @see CicadaTrackType
 */
@property (nonatomic, assign) CicadaTrackType trackType;

@property(nonatomic, assign) CicadaVideoHDRType HDRType;

/**
 @brief vod format
 */
@property (nonatomic, copy) NSString* vodFormat;

/**
 @brief track index
 */
@property (nonatomic, assign) int trackIndex;

/**
 @brief description
 */
@property (nonatomic, copy) NSString* description;

/**
 @brief video width, only meaningful if the trackType is CICADA_TRACK_VIDEO
 */
@property (nonatomic, assign) int videoWidth;

/**
 @brief video height, only meaningful if the trackType is CICADA_TRACK_VIDEO
 */
@property (nonatomic, assign) int videoHeight;

/**
 @brief bitrate, only meaningful if the trackType is CICADA_TRACK_VIDEO
 */
@property (nonatomic, assign) int trackBitrate;

/**
 @brief the number of audio channel, only meaningful if the trackType is CICADA_TRACK_AUDIO
 */
@property (nonatomic, assign) int audioChannels;

/**
 @brief audio samplerate, only meaningful if the trackType is CICADA_TRACK_AUDIO
 */
@property (nonatomic, assign) int audioSamplerate;

/**
 @brief audio sampleFormat, only meaningful if the trackType is CICADA_TRACK_AUDIO
 */
@property (nonatomic, assign) int audioSampleFormat;

/**
 @brief audio Language, only meaningful if the trackType is CICADA_TRACK_AUDIO
 */
@property (nonatomic, copy) NSString* audioLanguage;

/**
 @brief subtitle Language, only meaningful if the trackType is CICADA_TRACK_SUBTITLE
 */
@property (nonatomic, copy) NSString* subtitleLanguage;


@end


OBJC_EXPORT
@interface CicadaThumbnailInfo : NSObject

/**
 @brief thumbnail URL
 */
@property (nonatomic, copy) NSString* URL;

@end


OBJC_EXPORT
@interface CicadaMediaInfo : NSObject

/**
 @brief bitrate
 */
@property(nonatomic, assign) int64_t totalBitrate;

/**
 @brief tracks in media info
 */
@property (nonatomic, strong) NSArray<CicadaTrackInfo*>* tracks;

@end

#endif /* CicadaMediaInfo_h */
