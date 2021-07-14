
#include "CicadaOCHelper.h"
#include <EventCodeMap.h>
#include <ErrorCodeMap.h>

using namespace Cicada;

CicadaImage * CicadaOCHelper::convertBitmapRGBA8ToUIImage(unsigned char *buffer, int width, int height) {
    size_t bufferLength = width * height * 4;
    CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, buffer, bufferLength, NULL);
    size_t bitsPerComponent = 8;
    size_t bitsPerPixel = 32;
    size_t bytesPerRow = 4 * width;

    CGColorSpaceRef colorSpaceRef = CGColorSpaceCreateDeviceRGB();
    if(colorSpaceRef == NULL) {
        NSLog(@"Error allocating color space");
        CGDataProviderRelease(provider);
        return nil;
    }

    CGBitmapInfo bitmapInfo = kCGBitmapByteOrderDefault;
    CGColorRenderingIntent renderingIntent = kCGRenderingIntentDefault;

    CGImageRef iref = CGImageCreate(width,
                                    height,
                                    bitsPerComponent,
                                    bitsPerPixel,
                                    bytesPerRow,
                                    colorSpaceRef,
                                    bitmapInfo,
                                    provider,    // data provider
                                    NULL,        // decode
                                    YES,            // should interpolate
                                    renderingIntent);

    uint32_t* pixels = (uint32_t*)malloc(bufferLength);

    if (pixels == NULL) {
        NSLog(@"Error: Memory not allocated for bitmap");
        CGDataProviderRelease(provider);
        CGColorSpaceRelease(colorSpaceRef);
        CGImageRelease(iref);
        return nil;
    }

    CGContextRef context = CGBitmapContextCreate(pixels,
                                                 width,
                                                 height,
                                                 bitsPerComponent,
                                                 bytesPerRow,
                                                 colorSpaceRef,
                                                 kCGImageAlphaPremultipliedLast);

    CicadaImage *image = nil;
    if (context) {

        CGContextDrawImage(context, CGRectMake(0.0f, 0.0f, width, height), iref);

        CGImageRef imageRef = CGBitmapContextCreateImage(context);

#if TARGET_OS_OSX
        NSSize size;
        size.height = height;
        size.width = width;
        image = [[NSImage alloc] initWithCGImage:imageRef size:size];
#else
        // Support both iPad 3.2 and iPhone 4 Retina displays with the correct scale
        if([UIImage respondsToSelector:@selector(imageWithCGImage:scale:orientation:)]) {
            float scale = [[UIScreen mainScreen] scale];
            image = [UIImage imageWithCGImage:imageRef scale:scale orientation:UIImageOrientationUp];
        } else {
            image = [UIImage imageWithCGImage:imageRef];
        }
#endif
        CGImageRelease(imageRef);
        CGContextRelease(context);
    }

    CGColorSpaceRelease(colorSpaceRef);
    CGImageRelease(iref);
    CGDataProviderRelease(provider);

    if (pixels) {
        free(pixels);
    }

    return image;
}

CicadaTrackInfo* CicadaOCHelper::getCicadaTrackInfo(const StreamInfo *info)
{
    if (nullptr == info) {
        return nil;
    }
    CicadaTrackInfo* trackInfo = [[CicadaTrackInfo alloc] init];

    trackInfo.trackType = static_cast<CicadaTrackType>(info->type);
    trackInfo.trackIndex = info->streamIndex;
    if (info->description) {
        trackInfo.description = [NSString stringWithUTF8String:info->description];
    }
    switch (trackInfo.trackType) {
        case CICADA_TRACK_VIDEO:
            trackInfo.videoWidth = info->videoWidth;
            trackInfo.videoHeight = info->videoHeight;
            trackInfo.trackBitrate = info->videoBandwidth;
            trackInfo.HDRType = static_cast<CicadaVideoHDRType>(info->HDRType);
            break;
        case CICADA_TRACK_AUDIO:
            trackInfo.audioChannels = info->nChannels;
            trackInfo.audioSamplerate = info->sampleRate;
            trackInfo.audioSampleFormat = info->sampleFormat;
            if (info->audioLang) {
                trackInfo.audioLanguage = [NSString stringWithUTF8String:info->audioLang];
            }
            break;
        case CICADA_TRACK_SUBTITLE:
            if (info->subtitleLang) {
                trackInfo.subtitleLanguage = [NSString stringWithUTF8String:info->subtitleLang];
            }
            break;
        default:
            break;
    }

    return trackInfo;
}

CicadaStatus CicadaOCHelper::mapStatus(int64_t status) {
    if (status == PLAYER_IDLE) {
        return CicadaStatusIdle;
    } else if (status == PLAYER_INITIALZED) {
        return CicadaStatusInitialzed;
    } else if (status == PLAYER_PREPARED) {
        return CicadaStatusPrepared;
    } else if (status == PLAYER_PLAYING) {
        return CicadaStatusStarted;
    } else if (status == PLAYER_PAUSED) {
        return CicadaStatusPaused;
    } else if (status == PLAYER_STOPPED) {
        return CicadaStatusStopped;
    } else if (status == PLAYER_COMPLETION) {
        return CicadaStatusCompletion;
    } else if (status == PLAYER_ERROR) {
        return CicadaStatusError;
    } else {
        return CicadaStatusMax;
    }
}

void CicadaOCHelper::getListener(playerListener &listener)
{
    listener.userData = this;
    listener.LoopingStart = onCircleStart;
    listener.AutoPlayStart = onAutoPlayStart;
    listener.FirstFrameShow = onFirstFrameShow;
    listener.VideoSizeChanged = onVideoSizeChanged;
    listener.PositionUpdate = onCurrentPositionUpdate;
    listener.BufferPositionUpdate = onBufferPositionUpdate;
    listener.LoadingStart = onLoadingStart;
    listener.LoadingProgress = onLoadingProgress;
    listener.CurrentDownLoadSpeed = onCurrentDownLoadSpeed;
    listener.LoadingEnd = onLoadingEnd;
    listener.SeekEnd = onSeekEnd;
    listener.MediaInfoGet = onMediaInfoGet;
    listener.StreamSwitchSuc = onSwitchStreamSuccess;
    listener.SubtitleExtAdd = onSubtitleExtAdd;
    listener.StatusChanged = onPlayerStatusChanged;
    listener.CaptureScreen = onCaptureScreen;
    listener.SubtitleShow = onShowSubtitle;
    listener.SubtitleHide = onHideSubtitle;
    listener.SubtitleHeader = onSubtitleHeader;
    listener.VideoRendered = onVideoRendered;
    listener.AudioRendered = onAudioRendered;
    listener.EventCallback = onEvent;
    listener.ErrorCallback = onError;
    listener.Prepared = onPrepared;
    listener.Completion = onCompletion;
}

CicadaPlayer * CicadaOCHelper::getOCPlayer(void *userData)
{
    CicadaOCHelper *helper = (CicadaOCHelper *)userData;
    if (nullptr == helper) {
        return nil;
    }
    return helper->mPlayer;
}

id<CicadaDelegate> CicadaOCHelper::getDelegate(void *userData)
{
    CicadaOCHelper *helper = (CicadaOCHelper *)userData;
    if (nullptr == helper) {
        return nil;
    }
    return helper->mInnerDelegate;
}

void CicadaOCHelper::onPrepared(void *userData) {
    __weak CicadaPlayer * player = getOCPlayer(userData);
    __weak id<CicadaDelegate> theDelegate = getDelegate(userData);

    dispatch_async(dispatch_get_main_queue(), ^{
        player.duration = 0; // setDuration will overwrite the value
        if (theDelegate && [theDelegate respondsToSelector:@selector(onPlayerEvent:eventType:)]) {
            [theDelegate onPlayerEvent:player eventType:CicadaEventPrepareDone];
        }
        
        if (player.delegate && [player.delegate respondsToSelector:@selector(onPlayerEvent:eventType:)]){
            [player.delegate onPlayerEvent:player eventType:CicadaEventPrepareDone];
        }
    });
}

void CicadaOCHelper::onCompletion(void *userData) {
    __weak CicadaPlayer * player = getOCPlayer(userData);
    if (player.delegate && [player.delegate respondsToSelector:@selector(onPlayerEvent:eventType:)]) {
        dispatch_async(dispatch_get_main_queue(), ^{

            [player.delegate onPlayerEvent:player eventType:CicadaEventCompletion];
        });
    }
}

void CicadaOCHelper::onFirstFrameShow(void *userData) {
    __weak CicadaPlayer * player = getOCPlayer(userData);
    if(player.delegate && [player.delegate respondsToSelector:@selector(onPlayerEvent:eventType:)]){
        dispatch_async(dispatch_get_main_queue(), ^{
            [player.delegate onPlayerEvent:player eventType:CicadaEventFirstRenderedStart];
        });
    }
}

void CicadaOCHelper::onCircleStart(void *userData) {
    __weak CicadaPlayer * player = getOCPlayer(userData);
    if(player.delegate && [player.delegate respondsToSelector:@selector(onPlayerEvent:eventType:)]){
        dispatch_async(dispatch_get_main_queue(), ^{
            [player.delegate onPlayerEvent:player eventType:CicadaEventLoopingStart];
        });
    }
}

void CicadaOCHelper::onError(int64_t code, const void *msg, void *userData) {
    __weak CicadaPlayer * player = getOCPlayer(userData);
    NSString* errorStr = [[NSString alloc] initWithUTF8String:(const char *)msg];
    if(player.delegate && [player.delegate respondsToSelector:@selector(onError:errorModel:)]){
        dispatch_async(dispatch_get_main_queue(), ^{
            CicadaErrorModel* model = [[CicadaErrorModel alloc] init];
            model.code = (CicadaErrorCode) ErrorCodeMap::getInstance().getValue(static_cast<int>(code));
            model.message = errorStr;
            [player.delegate onError:player errorModel:model];
        });
    }
}

void CicadaOCHelper::onVideoSizeChanged(int64_t width, int64_t height, void *userData) {
    __weak CicadaPlayer * player = getOCPlayer(userData);
    dispatch_async(dispatch_get_main_queue(), ^{
        [player setWidth:(int)width];
        [player setHeight:(int)height];
        //TODO: rotation value
        if (player.delegate && [player.delegate respondsToSelector:@selector(onVideoSizeChanged:width:height:rotation:)]){
            [player.delegate onVideoSizeChanged:player width:(int)width height:(int)height rotation:0.0f];
        }
    });
}

void CicadaOCHelper::onCurrentPositionUpdate(int64_t position, void *userData) {
    __weak CicadaPlayer * player = getOCPlayer(userData);
    __weak id<CicadaDelegate> theDelegate = getDelegate(userData);
    dispatch_async(dispatch_get_main_queue(), ^{
        [player setCurrentPosition:position];
        if (player.delegate && [player.delegate respondsToSelector:@selector(onCurrentPositionUpdate:position:)]) {
            [player.delegate onCurrentPositionUpdate:player position:position];
        }
        if (theDelegate && [theDelegate respondsToSelector:@selector(onCurrentPositionUpdate:position:)]) {
            [theDelegate onCurrentPositionUpdate:player position:position];
        }
    });
}

void CicadaOCHelper::onBufferPositionUpdate(int64_t position, void *userData) {
    __weak CicadaPlayer * player = getOCPlayer(userData);
    dispatch_async(dispatch_get_main_queue(), ^{
        [player setBufferedPosition:position];
        if (player.delegate && [player.delegate respondsToSelector:@selector(onBufferedPositionUpdate:position:)]) {
            [player.delegate onBufferedPositionUpdate:player position:position];
        }
    });
}

void CicadaOCHelper::onLoadingStart(void *userData) {
    __weak CicadaPlayer * player = getOCPlayer(userData);
    if(player.delegate && [player.delegate respondsToSelector:@selector(onPlayerEvent:eventType:)]){
        dispatch_async(dispatch_get_main_queue(), ^{
            [player.delegate onPlayerEvent:player eventType:CicadaEventLoadingStart];
        });
    }
}

void CicadaOCHelper::onLoadingEnd(void *userData)     {
    __weak CicadaPlayer * player = getOCPlayer(userData);
    if(player.delegate && [player.delegate respondsToSelector:@selector(onPlayerEvent:eventType:)]){
        dispatch_async(dispatch_get_main_queue(), ^{
            [player.delegate onPlayerEvent:player eventType:CicadaEventLoadingEnd];
        });
    }
}

void CicadaOCHelper::onLoadingProgress(int64_t percent, void *userData) {
    __weak CicadaPlayer * player = getOCPlayer(userData);
    if(player.delegate && [player.delegate respondsToSelector:@selector(onLoadingProgress:progress:)]){
        dispatch_async(dispatch_get_main_queue(), ^{
            [player.delegate onLoadingProgress:player progress:percent];
        });
    }
}

void CicadaOCHelper::onCurrentDownLoadSpeed(int64_t speed, void *userData)
{
    __weak CicadaPlayer *player = getOCPlayer(userData);
    if (player.delegate && [player.delegate respondsToSelector:@selector(onCurrentDownLoadSpeed:speed:)]) {
        dispatch_async(dispatch_get_main_queue(), ^{
          [player.delegate onCurrentDownLoadSpeed:player progress:speed];
        });
    }
}

#define ASSColorFromVB(rgbValue)                                                                                                           \
    [UIColor colorWithRed:((float) ((rgbValue & 0xFF000000) >> 24)) / 255.0                                                                \
                    green:((float) ((rgbValue & 0xFF0000) >> 16)) / 255.0                                                                  \
                     blue:((float) ((rgbValue & 0xFF00) >> 8)) / 255.0                                                                     \
                    alpha:(1.0 - ((float) (rgbValue & 0xFF)) / 255.0)];

#define ASSColorFromBGR(bgrValue)                                                                                                          \
    [UIColor colorWithRed:((float) (bgrValue & 0xFF)) / 255.0                                                                              \
                    green:((float) ((bgrValue & 0xFF00) >> 8)) / 255.0                                                                     \
                     blue:((float) ((bgrValue & 0xFF0000) >> 16)) / 255.0                                                                  \
                    alpha:1.0];

void CicadaOCHelper::onShowSubtitle(int64_t index, int64_t size, const void *data, void *userData) {
    __weak CicadaPlayer * player = getOCPlayer(userData);

    if (nullptr == data) return;
    IAFPacket *packet = (IAFPacket *) (data);
    int subtitleIndex = packet->getInfo().streamIndex;
    int64_t pts = packet->getInfo().pts;
    NSData* stringData = [[NSData alloc] initWithBytes:packet->getData() length:(unsigned int)packet->getSize()];
    NSString* str = [[NSString alloc] initWithData:stringData encoding:NSUTF8StringEncoding];

    CicadaOCHelper *helper = (CicadaOCHelper *) userData;
    if (helper->assHeader.Type == SubtitleTypeAss) {
#if TARGET_OS_IPHONE
        if ([str length] > 0) {
            AssDialogue ret = AssUtils::parseAssDialogue(helper->assHeader, [str UTF8String]);

            dispatch_async(dispatch_get_main_queue(), ^{
              CATextLayer *textLayer = nil;
              NSString *layerKey = [NSString stringWithFormat:@"%i", ret.Layer];
              if ([helper->layerDic objectForKey:layerKey]) {
                  textLayer = [helper->layerDic objectForKey:layerKey];
              } else {
                  textLayer = [CATextLayer layer];
                  //                  textLayer.frame = player.playerView.bounds;
                  textLayer.contentsScale = [UIScreen mainScreen].scale;
                  textLayer.wrapped = YES;
                  [player.playerView.layer insertSublayer:textLayer atIndex:ret.Layer + 1];

                  [helper->layerDic setValue:textLayer forKey:layerKey];
              }

              textLayer.hidden = NO;

              helper->buildAssStyle(textLayer, ret, userData);
            });
            //        NSLog(@"=====%s",ret.Text.c_str());
        }
#endif
    } else {
        if (player.delegate && [player.delegate respondsToSelector:@selector(onSubtitleShow:trackIndex:subtitleID:subtitle:)]) {
            dispatch_async(dispatch_get_main_queue(), ^{
              [player.delegate onSubtitleShow:player trackIndex:(int) subtitleIndex subtitleID:pts subtitle:str];
            });
        }
    }
}

void
CicadaOCHelper::onCaptureScreen(int64_t width, int64_t height, const void *buffer, void *userData) {
    __weak CicadaPlayer * player = getOCPlayer(userData);
    if (nullptr == buffer) {
        return;
    }
    CicadaImage *image = nullptr;
    if (width == -1 && height == -1) {
        image = (__bridge id) buffer;
    } else {
        image = convertBitmapRGBA8ToUIImage((unsigned char *) buffer, static_cast<int>(width), static_cast<int>(height));
    }

    if(player.delegate && [player.delegate respondsToSelector:@selector(onCaptureScreen:image:)]){
        dispatch_async(dispatch_get_main_queue(), ^{

            [player.delegate onCaptureScreen:player image:image];
        });
    }
}

void CicadaOCHelper::onHideSubtitle(int64_t index, int64_t size, const void *data, void *userData)
{
    __weak CicadaPlayer * player = getOCPlayer(userData);
    if (nullptr == data) return;
    IAFPacket *packet = (IAFPacket *) (data);
    int subtitleIndex = packet->getInfo().streamIndex;
    int64_t pts = packet->getInfo().pts;

    CicadaOCHelper *helper = (CicadaOCHelper *) userData;
    if (helper->assHeader.Type == SubtitleTypeAss) {
#if TARGET_OS_IPHONE
        NSData *stringData = [[NSData alloc] initWithBytes:packet->getData() length:(unsigned int) packet->getSize()];
        NSString *str = [[NSString alloc] initWithData:stringData encoding:NSUTF8StringEncoding];
        if ([str length] > 0) {
            AssDialogue ret = AssUtils::parseAssDialogue(helper->assHeader, [str UTF8String]);
            NSString *layerKey = [NSString stringWithFormat:@"%i", ret.Layer];
            UIView *assLab = [helper->layerDic objectForKey:layerKey];
            if (assLab) {
                dispatch_async(dispatch_get_main_queue(), ^{
                  assLab.hidden = YES;
                });
            }
        }
#endif
    } else {
        if (player.delegate && [player.delegate respondsToSelector:@selector(onSubtitleHide:trackIndex:subtitleID:)]) {
            dispatch_async(dispatch_get_main_queue(), ^{
              [player.delegate onSubtitleHide:player trackIndex:(int) subtitleIndex subtitleID:pts];
            });
        }
    }
}

void CicadaOCHelper::onSubtitleExtAdd(int64_t index, const void *url, void *userData)
{
    __weak CicadaPlayer * player = getOCPlayer(userData);
    NSString *str = [NSString stringWithUTF8String:(const char *)url];

    if (player.delegate && [player.delegate respondsToSelector:@selector(onSubtitleExtAdded:trackIndex:URL:)]) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [player.delegate onSubtitleExtAdded:player trackIndex:(int)index URL:str];
        });
    }
}

void CicadaOCHelper::onSubtitleHeader(int64_t index, const void *header, void *userData)
{
    __weak CicadaPlayer *player = getOCPlayer(userData);
    NSString *str = [NSString stringWithUTF8String:(const char *) header];

    if (player.delegate && [player.delegate respondsToSelector:@selector(onSubtitleHeader:trackIndex:Header:)]) {
        dispatch_async(dispatch_get_main_queue(), ^{
          [player.delegate onSubtitleHeader:player trackIndex:(int) index Header:str];
        });
    } else {
        // TODO:
        /*
         * 1. detect whether a ass header
         * 2. create a subtitle layer and add to mView
         */
        CicadaOCHelper *helper = (CicadaOCHelper *) userData;
        helper->assHeader = AssUtils::parseAssHeader([str UTF8String]);
        if (helper->assHeader.Type == SubtitleTypeAss) {
            helper->layerDic = @{}.mutableCopy;
        }
    }
}

NSArray *CicadaOCHelper::matchStringWithRegx(NSString *string, NSString *regexStr)
{

    NSRegularExpression *regex = [NSRegularExpression regularExpressionWithPattern:regexStr
                                                                           options:NSRegularExpressionCaseInsensitive
                                                                             error:nil];

    NSArray *matches = [regex matchesInString:string options:0 range:NSMakeRange(0, [string length])];

    NSMutableArray *array = [NSMutableArray array];

    for (NSTextCheckingResult *match in matches) {

        for (int i = 0; i < [match numberOfRanges]; i++) {
            NSString *component = [string substringWithRange:[match rangeAtIndex:i]];

            [array addObject:component];
        }
    }

    return array;
}

void CicadaOCHelper::buildAssStyle(CATextLayer *textLayer, AssDialogue ret, void *userData)
{
    __weak CicadaPlayer *player = getOCPlayer(userData);
    CicadaOCHelper *helper = (CicadaOCHelper *) userData;

    //TODO detect whether it is the default style
    std::map<std::string, AssStyle> styles = helper->assHeader.styles;
    std::map<std::string, AssStyle>::iterator iter = styles.begin();
    AssStyle assStyle = iter->second;

    NSString *fontName = [NSString stringWithCString:assStyle.FontName.c_str() encoding:NSUTF8StringEncoding];
    NSString *subtitle = [NSString stringWithCString:ret.Text.c_str() encoding:NSUTF8StringEncoding];

    subtitle = [subtitle stringByReplacingOccurrencesOfString:@"\\N" withString:@"\n"];

    //    NSLog(@"====%@",subtitle);

    NSArray *lineCodes = helper->matchStringWithRegx(subtitle, @"\\{[^\\{]+\\}");

    //TODO not for `p0` for the time being
    NSMutableAttributedString *attributedStr = [[NSMutableAttributedString alloc] init];
    if (lineCodes.count > 0) {
        if ([subtitle hasSuffix:@"p0}"]) {
            subtitle = @"";
        } else {
            NSString *preStyle = @"";
            for (int i = 0; i < lineCodes.count; i++) {
                NSString *code = [lineCodes objectAtIndex:i];
                NSRange range = [subtitle rangeOfString:code];
                NSUInteger end = 0;
                if (lineCodes.count > i + 1) {
                    NSString *nextCode = [lineCodes objectAtIndex:i + 1];
                    NSRange nextRange = [subtitle rangeOfString:nextCode];
                    end = nextRange.location;
                } else {
                    end = [subtitle length];
                }
                NSUInteger begin = range.location + range.length;
                NSString *text = [subtitle substringWithRange:NSMakeRange(begin, end - begin)];
                if (text.length > 0) {
                    if (preStyle.length > 0) {
                        code = [preStyle stringByAppendingString:code];
                        preStyle = @"";
                    }
                    [attributedStr appendAttributedString:helper->buildAssStyleStr(code, text, assStyle, userData)];
                } else {
                    preStyle = [preStyle stringByAppendingString:code];
                }

                if (subtitle.length > end) {
                    subtitle = [subtitle substringFromIndex:end];
                }
            }
        }
    } else {
        [attributedStr appendAttributedString:helper->buildAssStyleStr(nil, subtitle, assStyle, userData)];
    }

    CGFloat x = 0;
    CGFloat y = 0;
    CGFloat w = CGRectGetWidth(player.playerView.frame);
    CGFloat h = CGRectGetHeight(player.playerView.frame);

    CGSize textSize = CGSizeZero;
    if (attributedStr.length > 0) {
        textLayer.string = attributedStr;
        textSize = helper->getSubTitleHeight(attributedStr, w);
    }

    switch (assStyle.Alignment % 4) {
        //align left
        case 1:
            x = 0;
            x += assStyle.MarginL;
            textLayer.alignmentMode = kCAAlignmentLeft;
            break;
        //Center
        case 2:
            //            x = (w - textSize.width) / 2;
            textLayer.alignmentMode = kCAAlignmentCenter;
            break;
        //align right
        case 3:
            x = w - textSize.width;
            x -= assStyle.MarginR;
            textLayer.alignmentMode = kCAAlignmentRight;
            break;

        default:
            break;
    }
    switch (assStyle.Alignment / 4) {
        //bottom
        case 0:
            y = h - textSize.height;
            y -= assStyle.MarginV;
            break;
        //top
        case 1:
            y = 0;
            x += assStyle.MarginV;
            break;
        //Center
        case 2:
            y = (h - textSize.height) / 2;
            break;

        default:
            break;
    }

    textLayer.frame = CGRectMake(x, y, textSize.width, textSize.height);
}

NSAttributedString *CicadaOCHelper::buildAssStyleStr(NSString *style, NSString *text, AssStyle defaultstyle, void *userData)
{
    CicadaOCHelper *helper = (CicadaOCHelper *) userData;

    NSMutableDictionary<NSAttributedStringKey, id> *attrs = @{}.mutableCopy;

    NSString *fontName = [NSString stringWithCString:defaultstyle.FontName.c_str() encoding:NSUTF8StringEncoding];
    int fontSize = defaultstyle.FontSize;

    UIColor *color = ASSColorFromVB(defaultstyle.PrimaryColour);
    ;
    [attrs setValue:color forKey:NSForegroundColorAttributeName];

    if (style) {
        NSArray *styleArr = [style componentsSeparatedByCharactersInSet:[NSCharacterSet characterSetWithCharactersInString:@"{}"]];
        for (NSString *subStr in styleArr) {
            NSArray *subStyleArr = [subStr componentsSeparatedByString:@"\\"];
            for (NSString *item in subStyleArr) {
                NSString *itemStr = [item stringByReplacingOccurrencesOfString:@" " withString:@""];
                if ([itemStr hasPrefix:@"fn"]) {
                    fontName = [itemStr substringFromIndex:@"fn".length];
                } else if ([itemStr hasPrefix:@"fs"]) {
                    fontSize = [itemStr substringFromIndex:@"fs".length].intValue;
                } else if (helper->matchStringWithRegx(itemStr, @"^b[0-9]+$").count > 0) {
                    itemStr = [itemStr stringByReplacingOccurrencesOfString:@"b" withString:@""];
                    [attrs setValue:[NSNumber numberWithInt:itemStr.intValue] forKey:NSExpansionAttributeName];
                } else if (helper->matchStringWithRegx(itemStr, @"^i[0-9]+$").count > 0) {
                    itemStr = [itemStr stringByReplacingOccurrencesOfString:@"i" withString:@""];
                    [attrs setValue:[NSNumber numberWithInt:itemStr.intValue] forKey:NSObliquenessAttributeName];
                } else if (helper->matchStringWithRegx(itemStr, @"^u[0-9]+$").count > 0) {
                    itemStr = [itemStr stringByReplacingOccurrencesOfString:@"u" withString:@""];
                    [attrs setValue:[NSNumber numberWithInt:itemStr.intValue] forKey:NSUnderlineStyleAttributeName];
                } else if (helper->matchStringWithRegx(itemStr, @"^s[0-9]+$").count > 0) {
                    itemStr = [itemStr stringByReplacingOccurrencesOfString:@"s" withString:@""];
                    [attrs setValue:[NSNumber numberWithInt:itemStr.intValue] forKey:NSStrikethroughStyleAttributeName];
                } else if ([itemStr hasPrefix:@"c&H"] || [itemStr hasPrefix:@"1c&H"]) {
                    NSRange range = [itemStr rangeOfString:@"c&H"];
                    itemStr = [itemStr substringFromIndex:range.location + range.length];
                    itemStr = [itemStr stringByReplacingOccurrencesOfString:@"&" withString:@""];
                    unsigned colorInt = 0;
                    [[NSScanner scannerWithString:itemStr] scanHexInt:&colorInt];
                    UIColor *color = ASSColorFromBGR(colorInt);
                    [attrs setValue:color forKey:NSForegroundColorAttributeName];
                }
            }
        }
    }

    UIFont *font = [UIFont fontWithName:fontName size:fontSize];

    [attrs setValue:font ?: [UIFont systemFontOfSize:fontSize] forKey:NSFontAttributeName];

    return [[NSAttributedString alloc] initWithString:text attributes:attrs];
}

CGSize CicadaOCHelper::getSubTitleHeight(NSMutableAttributedString *attrStr, CGFloat viewWidth)
{
    CGSize textSize = CGSizeZero;
    if (attrStr.length == 0) {
        return textSize;
    }

    //    CGSize size  = [attrStr boundingRectWithSize:CGSizeMake(viewWidth, MAXFLOAT) options: NSStringDrawingUsesLineFragmentOrigin | NSStringDrawingUsesFontLeading  context:nil].size;
    NSMutableParagraphStyle *paragraphStyle = [NSMutableParagraphStyle new];
    //    [paragraphStyle setLineSpacing:lineSpace];
    paragraphStyle.lineBreakMode = NSLineBreakByCharWrapping;
    NSRange range = NSMakeRange(0, attrStr.length);
    [attrStr addAttribute:NSParagraphStyleAttributeName value:paragraphStyle range:range];

    NSDictionary *dic = [attrStr attributesAtIndex:0 effectiveRange:&range];
    textSize = [attrStr.string boundingRectWithSize:CGSizeMake(viewWidth, MAXFLOAT)
                                            options:NSStringDrawingUsesLineFragmentOrigin | NSStringDrawingUsesFontLeading
                                         attributes:dic
                                            context:nil]
                       .size;
    return CGSizeMake(viewWidth, textSize.height);
}
#endif

void
CicadaOCHelper::onSwitchStreamSuccess(int64_t type, const void *item, void *userData) {
    __weak CicadaPlayer * player = getOCPlayer(userData);
    const auto *newStream = static_cast<const StreamInfo *>(item);
    int streamIndex = newStream->streamIndex;
    if (player.delegate && [player.delegate respondsToSelector:@selector(onTrackChanged:info:)]){
        dispatch_async(dispatch_get_main_queue(), ^{

            CicadaMediaInfo* mediaInfo = [player getMediaInfo];
            for (CicadaTrackInfo* track in mediaInfo.tracks) {
                if (streamIndex == track.trackIndex) {
                    if (track.trackType == type) {
                        [player.delegate onTrackChanged:player info:track];
                        break;
                    }
                }
            }
        });
    }
}

void CicadaOCHelper::onMediaInfoGet(int64_t count, const void *infos, void *userData)
{
    __weak CicadaPlayer * player = getOCPlayer(userData);
    CicadaMediaInfo* mediaInfo = [player getMediaInfo];

    if (nullptr != infos) {
        MediaInfo *cMediaInfo = (MediaInfo *) infos;
        mediaInfo.totalBitrate = cMediaInfo->totalBitrate;
        auto &streamInfos = cMediaInfo->mStreamInfoQueue;
        NSMutableArray* tracks = [[NSMutableArray alloc] init];
        for (int i = 0; i < count; ++i) {
            CicadaTrackInfo* trackInfo = getCicadaTrackInfo(streamInfos[i]);
            [tracks addObject:trackInfo];
        }

        mediaInfo.tracks = tracks;

        if ([tracks count] > 0) {
            if(player.delegate && [player.delegate respondsToSelector:@selector(onTrackReady:info:)]){
                dispatch_async(dispatch_get_main_queue(), ^{

                    [player.delegate onTrackReady:player info:tracks];
                });
            }
        }
    }
}

void CicadaOCHelper::onSeekEnd(int64_t seekInCache, void *userData) {
    __weak CicadaPlayer * player = getOCPlayer(userData);
    if (player && player.delegate && [player.delegate respondsToSelector:@selector(onPlayerEvent:eventType:)]) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [player.delegate onPlayerEvent:player eventType:CicadaEventSeekEnd];
        });
    }
}

void CicadaOCHelper::onAutoPlayStart(void *userData) {
    __weak CicadaPlayer * player = getOCPlayer(userData);
    if (player && player.delegate && [player.delegate respondsToSelector:@selector(onPlayerEvent:eventType:)]) {
        dispatch_async(dispatch_get_main_queue(), ^{

            [player.delegate onPlayerEvent:player eventType:CicadaEventAutoPlayStart];
        });
    }
}

void CicadaOCHelper::onPlayerStatusChanged(int64_t oldStatus, int64_t newStatus, void *userData) {
    __weak CicadaPlayer * player = getOCPlayer(userData);
    __weak id<CicadaDelegate> theDelegate = getDelegate(userData);
    dispatch_async(dispatch_get_main_queue(), ^{
        if (theDelegate && [theDelegate respondsToSelector:@selector(onPlayerStatusChanged:oldStatus:newStatus:)]) {
            [theDelegate onPlayerStatusChanged:player oldStatus:mapStatus(oldStatus) newStatus:mapStatus(newStatus)];
        }

        if (player && player.delegate && [player.delegate respondsToSelector:@selector(onPlayerStatusChanged:oldStatus:newStatus:)]) {
            [player.delegate onPlayerStatusChanged:player oldStatus:mapStatus(oldStatus) newStatus:mapStatus(newStatus)];
        }
    });
}

void CicadaOCHelper::onEvent(int64_t code, const void *msg, void *userData) {
    __weak CicadaPlayer * player = getOCPlayer(userData);
    NSString* str = @"";
    if (msg != nullptr) {
        str = [[NSString alloc] initWithUTF8String:(const char *)msg];
    }

    if (player.delegate && [player.delegate respondsToSelector:@selector(onPlayerEvent:eventWithString:description:)]) {
        dispatch_async(dispatch_get_main_queue(), ^{
          CicadaEventWithString eventCode = (CicadaEventWithString) EventCodeMap::getInstance().getValue(static_cast<int>(code));
          [player.delegate onPlayerEvent:player eventWithString:(CicadaEventWithString) eventCode description:str];
        });
    }
}

void CicadaOCHelper::onVideoRendered(int64_t theTimeMs, int64_t thePts, void *userData) {
    __weak CicadaPlayer * player = getOCPlayer(userData);

    if (player.delegate && [player.delegate respondsToSelector:@selector(onVideoRendered:timeMs:pts:)]) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [player.delegate onVideoRendered:player timeMs:theTimeMs pts:thePts];
        });
    }
}

void CicadaOCHelper::onAudioRendered(int64_t theTimeMs, int64_t thePts, void *userData)
{
    __weak CicadaPlayer *player = getOCPlayer(userData);

    if (player.delegate && [player.delegate respondsToSelector:@selector(onAudioRendered:timeMs:pts:)]) {
        dispatch_async(dispatch_get_main_queue(), ^{
          [player.delegate onAudioRendered:player timeMs:theTimeMs pts:thePts];
        });
    }
}
