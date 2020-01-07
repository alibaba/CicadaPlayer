
#import "CicadaThumbnail.h"
#import "utils/CicadaThumbnailParser.h"
#import "CicadaThumbnailItem.h"

#import <TargetConditionals.h>
#import "CicadaURLSession.h"

typedef NS_ENUM(NSUInteger, THUMB_INIT_STATUS) {
    THUMB_INIT_INITIALIZE,
    THUMB_INIT_SUCCESS,
    THUMB_INIT_FAIL
};

@interface CicadaThumbnail()
{
    CicadaImage *_tempThumbImage;
}

@property (retain, nonatomic) NSURL* thumbURL;

@property (retain, nonatomic) NSMutableArray<CicadaThumbnailItem *>* thumbArray;
@property (retain, atomic) NSMutableDictionary<NSString*, NSObject *>* dataMap;
@property (retain, nonatomic) NSMutableArray<NSNumber *>* thumbRequest;

@property (retain, nonatomic) NSLock * locker;

@property (retain, nonatomic) CicadaThumbnailItem * lastRequestItem;
@property (assign) int64_t lastPosition;
@property (assign) THUMB_INIT_STATUS initStatus;
@end

@implementation CicadaThumbnail

+ (void)dispatch_main_async_safe:(void(^)(void))block{
    if ([NSThread isMainThread]) {
        block();
    }else{
        dispatch_async(dispatch_get_main_queue(), block);
    }
}

#if TARGET_OS_IPHONE
-(CicadaImage *)ct_imageFromImage:(CicadaImage *)image inRect:(CGRect)rect
{
    CGImageRef sourceImageRef = [image CGImage];
    CGImageRef newImageRef = CGImageCreateWithImageInRect(sourceImageRef, rect);
    CicadaImage *newImage = [CicadaImage imageWithCGImage:newImageRef scale:[UIScreen mainScreen].scale orientation:UIImageOrientationUp];
    CGImageRelease(newImageRef);
    return newImage;
}
#else
-(CicadaImage *)ct_imageFromImage:(CicadaImage *)image inRect:(CGRect)rect
{
    // create the image somehow, load from file, draw into it...
    CGImageSourceRef source = CGImageSourceCreateWithData((CFDataRef)[image TIFFRepresentation], NULL);
    CGImageRef sourceImageRef =  CGImageSourceCreateImageAtIndex(source, 0, NULL);
    CGImageRef newImageRef = CGImageCreateWithImageInRect(sourceImageRef, rect);
    CicadaImage *newImage = [[NSImage alloc] initWithCGImage:newImageRef size:rect.size];
    CGImageRelease(newImageRef);
    CGImageRelease(sourceImageRef);
    return newImage;
}
#endif

- (instancetype _Nonnull )init:(NSURL *_Nonnull)URL
{
    if (nil != (self = [super init]))
    {
        self.initStatus = THUMB_INIT_INITIALIZE;
        self.thumbURL = URL;
        self.locker = [[NSLock alloc] init];
        self.thumbArray = [[NSMutableArray alloc] init];
        self.thumbRequest = [[NSMutableArray alloc] init];
        self.dataMap = [[NSMutableDictionary alloc] init];
        self.lastPosition = -1;
        [self parserThumbnails];
    }

    return self;
}

- (void) getAllThumbnails
{
    [self.thumbArray enumerateObjectsUsingBlock:^(CicadaThumbnailItem* _Nonnull info, NSUInteger idx, BOOL * _Nonnull stop) {

        // if user try to getThumbnail in THUMB_INIT_INITIALIZE
        [self.locker lock];
        if (0 <= self.lastPosition
            && (nil == self.lastRequestItem)
            && (self.lastPosition < info.thumbUntil)
            && (self.lastPosition >= info.thumbStart)) {
            // set lastRequestItem according to the position
            self.lastRequestItem = info;
        }
        [self.locker unlock];

        if (nil == self.dataMap[info.thumbPath]) {
            [self.dataMap setObject:[NSNull null] forKey:info.thumbPath];
            [self getURLImage:info];
        }
    }];
}

-(void) getURLImage:(CicadaThumbnailItem*)info
{
    NSURL *picURL = [NSURL URLWithString:info.thumbPath relativeToURL:self.thumbURL];
    CicadaURLSession *session = [[CicadaURLSession alloc] init];
    [session request:picURL complete:^(CicadaURLSession *session, NSData * _Nullable data) {
        if (nil != data) {
#if TARGET_OS_IPHONE
            CicadaImage *image = [CicadaImage imageWithData:data];
#else
            CicadaImage *image = [[NSImage alloc] initWithData:data];
#endif
            [self.dataMap setObject:image forKey:info.thumbPath];

            [self.locker lock];
            if (self.lastRequestItem.thumbPath == info.thumbPath) {
                [self notifyImage:image info:self.lastRequestItem position:self.lastPosition];
                self.lastRequestItem = nil;
            }
            [self.locker unlock];
        }
        else {
            [self.dataMap removeObjectForKey:info.thumbPath];

            [self.locker lock];
            if (self.lastRequestItem.thumbPath == info.thumbPath) {
                [self notifyFail:self.lastPosition];
                self.lastRequestItem = nil;
            }
            [self.locker unlock];
        }
        [session cancel];
    }];
}

- (void) parserThumbnails
{
    CicadaURLSession *session = [[CicadaURLSession alloc] init];
    [session request:self.thumbURL complete:^(CicadaURLSession *session, NSData * _Nullable data) {
        if (nil == data) {
            self.initStatus = THUMB_INIT_FAIL;
            return;
        }

        NSString * content = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
        CicadaThumbnailParser parser;
        list<ThumbnailInfo> listThumbnail = parser.setInputData([content UTF8String]);
        if (0 == listThumbnail.size()) {
            self.initStatus = THUMB_INIT_FAIL;
            return;
        }

        NSLog(@"listThumbnail: %ld", listThumbnail.size());

        for (ThumbnailInfo &info : listThumbnail) {
            CicadaThumbnailItem *item = [[CicadaThumbnailItem alloc] init];
            item.thumbStart = info.timestamp;
            item.thumbUntil = info.timestamp + info.duration;
            item.thumbLeft = info.posX;
            item.thumbTop = info.posY;
            item.thumbWidth = info.width;
            item.thumbHeight = info.height;
            item.thumbPath = [NSString stringWithUTF8String:info.URI.c_str()];
            [self.thumbArray addObject:item];
        }
        self.initStatus = THUMB_INIT_SUCCESS;
        [self getAllThumbnails];
        [session cancel];
    }];
}

- (void)getThumbnail:(int64_t)position {
    if (THUMB_INIT_FAIL == self.initStatus) {
        [self notifyFail:position];
        return;
    }
    else if (THUMB_INIT_INITIALIZE == self.initStatus) {
        self.lastPosition = position;
        return;
    }
    else {
        [self getThumbnailInner:position];
    }
}

- (void)getThumbnailInner:(int64_t)position
{
    [self.thumbArray enumerateObjectsUsingBlock:^(CicadaThumbnailItem * _Nonnull info, NSUInteger idx, BOOL * _Nonnull stop) {
        if (position < info.thumbUntil && position >= info.thumbStart) {
            *stop = YES;
            [self.locker lock];
            self.lastRequestItem = nil;
            self.lastPosition = position;

            NSObject *obj = self.dataMap[info.thumbPath];
            if (nil == obj) {
                [self notifyFail:position];
            }
            else if (obj == [NSNull null]) {
                // waiting for downloading
                self.lastRequestItem = info;
            }
            else {
                [self notifyImage:(CicadaImage *)obj info:info position:position];
            }

            [self.locker unlock];
        }
    }];
}

-(void) notifyFail:(int64_t)position {
    [CicadaThumbnail dispatch_main_async_safe:^{
        if (self.delegate && [self.delegate respondsToSelector:@selector(onGetThumbnailFailed:)]) {
            [self.delegate onGetThumbnailFailed:position];
        }
    }];
}

-(void) notifyImage:(CicadaImage *)image info:(CicadaThumbnailItem *)info position:(int64_t)position {
    [CicadaThumbnail dispatch_main_async_safe:^{
        CGRect rect = CGRectMake(info.thumbLeft, info.thumbTop, info.thumbWidth, info.thumbHeight);
        CicadaImage* retImage = [self ct_imageFromImage:image inRect:rect];
        if (retImage) {
            if (self.delegate && [self.delegate respondsToSelector:@selector(onGetThumbnailSuc:fromPos:toPos:image:)]) {
                [self.delegate onGetThumbnailSuc:position fromPos:info.thumbStart toPos:info.thumbUntil image:retImage];
            }
        }else{
            if (self.delegate && [self.delegate respondsToSelector:@selector(onGetThumbnailFailed:)]) {
                [self.delegate onGetThumbnailFailed:position];
            }
        }
    }];
}

-(void)dealloc
{
}

@end
