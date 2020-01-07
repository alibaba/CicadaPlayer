//
//  CicadaPlayerView.h
//  render
//
//  Created by huang_jiafa on 2019/02/13.
//  Copyright (c) 2019 Aliyun. All rights reserved.
//
#include <TargetConditionals.h>

#if TARGET_OS_OSX
#import <AppKit/AppKit.h>
#define CICADA_VIEW NSOpenGLView
#elif TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#define CICADA_VIEW UIView
#endif // TARGET_OS_OSX

#include <mutex>

@protocol CicadaPlayerViewDelegate
- (void)notifyResize;
@end

@interface CicadaPlayerView : CICADA_VIEW
{
}

#if TARGET_OS_OSX
// use in sub-thread for macOS, support retina also
@property (atomic) NSRect subBackingBounds;
#endif
@property (assign) id<CicadaPlayerViewDelegate> delegate;

- (std::mutex &)getMutex;

#if TARGET_OS_OSX
- (bool)isValid; // for macOS only, set as invalid when start LiveResize
#endif

@end
