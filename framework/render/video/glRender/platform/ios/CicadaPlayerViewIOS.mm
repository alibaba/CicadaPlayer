//
//  CicadaPlayerView.mm
//  render
//
//  Created by huang_jiafa on 2019/02/13.
//  Copyright (c) 2019 Aliyun. All rights reserved.
//

#import "CicadaPlayerView.h"

@implementation CicadaPlayerView

+ (Class)layerClass {
    return [CAEAGLLayer class];
}

- (instancetype)initWithFrame:(CGRect)frame
{
    if (self = [super initWithFrame:frame])
    {
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 80000
        if ( [UIScreen instancesRespondToSelector:@selector(nativeScale)] )
        {
            self.contentScaleFactor = [UIScreen mainScreen].nativeScale;
        }
        else
#endif
        {
            self.contentScaleFactor = [UIScreen mainScreen].scale;
        }
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
        eaglLayer.opaque = YES;
        eaglLayer.drawableProperties = @{ kEAGLDrawablePropertyRetainedBacking : @(NO),
                                          kEAGLDrawablePropertyColorFormat : kEAGLColorFormatRGBA8 };
    }
    return self;
}

-(void)layoutSubviews {
    [super layoutSubviews];
    if (nil != _delegate) {
        [_delegate notifyResize];
    }
}

- (std::mutex &)getMutex {
    static std::mutex mutex;
    return mutex;
}

@end
