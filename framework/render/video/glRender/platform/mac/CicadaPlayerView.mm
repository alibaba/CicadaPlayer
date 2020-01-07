//
//  AliPlayerView.mm
//  render
//
//  Created by huang_jiafa on 2019/02/13.
//  Copyright (c) 2019 Aliyun. All rights reserved.
//

#import "CicadaPlayerView.h"
#define LOGVIEW if(false)NSLog

@interface CicadaPlayerView () {
    std::mutex mGLMutex;
    // set to invalid when just start LiveResize, to avoid the screen be out of shape
    bool isValid;
}

@property(nonatomic, assign) NSDate* liveResizeStart;
@end

@implementation CicadaPlayerView

- (void) initProperty
{
    // Enable retina-support
    self.wantsBestResolutionOpenGLSurface = YES;
    // Enable layer-backed drawing of view
    self.wantsLayer = YES;
    isValid = true;
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    if (self = [super initWithCoder:coder])
    {
        [self initProperty];
    }
    return self;
}

- (instancetype)initWithFrame:(CGRect)frame
{
    if (self = [super initWithFrame:frame])
    {
        [self initProperty];
    }
    return self;
}

- (void)viewDidChangeBackingProperties
{
    [super viewDidChangeBackingProperties];

    LOGVIEW(@"AliPlayerView viewDidChangeBackingProperties");
    // Need to propagate information about retina resolution
    self.layer.contentsScale = self.window.backingScaleFactor;
    self.subBackingBounds = [self convertRectToBacking:self.bounds];
}

- (void)update
{
    {
        std::lock_guard<std::mutex> locker(mGLMutex);
        [super update];
    }

    LOGVIEW(@"AliPlayerView update");

    if (isValid) {
        dispatch_async(dispatch_get_main_queue(), ^{
            if (nil != _delegate) {
                [_delegate notifyResize];
            }
        });
    }
}

- (std::mutex &)getMutex {
    return mGLMutex;
}

- (void)resizeWithOldSuperviewSize:(NSSize)oldSize
{
    //LOGVIEW(@"AliPlayerView resizeWithOldSuperviewSize");
    [super resizeWithOldSuperviewSize:oldSize];
    self.subBackingBounds = [self convertRectToBacking:self.bounds];
}

- (bool)isValid
{
    if (!isValid && self.liveResizeStart) {
        NSDate *now = [NSDate date];
        NSTimeInterval delta = [now timeIntervalSinceDate:self.liveResizeStart] * 1000;
        if (delta > 100) {
            return true;
        }
    }
    
    return isValid;
}

- (void)viewWillStartLiveResize
{
    isValid = false;
    self.liveResizeStart = [NSDate date];
    LOGVIEW(@"AliPlayerView viewWillStartLiveResize");
    [super viewWillStartLiveResize];
}

/* A view receives viewWillEndLiveResize after the frame is last changed for a live resize. It is important to call [super viewDidEndLiveResize].
 */
- (void)viewDidEndLiveResize
{
    LOGVIEW(@"AliPlayerView viewDidEndLiveResize");
    [super viewDidEndLiveResize];
    self.liveResizeStart = nil;
    isValid = true;
}

@end
