
#import <Foundation/Foundation.h>
#import "CAApplicationCaller.h"
#import <QuartzCore/QuartzCore.h>

@interface NSObject (CADisplayLink)
+ (id)displayLinkWithTarget:(id)arg1 selector:(SEL)arg2;

- (void)addToRunLoop:(id)arg1 forMode:(id)arg2;

- (void)setFrameInterval:(int)interval;

- (void)invalidate;
@end

@implementation CAApplicationCaller

@synthesize interval;

- (instancetype)init:(IVSync::Listener *)syncLisener
{
    if (self = [super init])
    {
        interval = 1;
        lisener = syncLisener;
    }
    return self;
}

- (void)dealloc {
    [super dealloc];
}

- (void)startMainLoop {
    dispatch_async(dispatch_get_main_queue(), ^{
        if (lisener) {
            lisener->VSyncOnInit();
        }
    });

    // CCDirector::setAnimationInterval() is called, we should invalidate it first
    [displayLink invalidate];
    displayLink = nil;
    displayLink = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget:self selector:@selector(displayLinkCallback:)];
  //  [displayLink setFrameInterval:self.interval];
    [displayLink addToRunLoop:[NSRunLoop mainRunLoop] forMode:NSDefaultRunLoopMode];
}

- (void) stop {
    dispatch_async(dispatch_get_main_queue(), ^{
        if (lisener) {
            lisener->VSyncOnDestroy();
        }
    });
    [displayLink release];
    displayLink = nil;
}

- (void)displayLinkCallback:(CADisplayLink *)sender {
    if (lisener) {
        lisener->onVSync([sender timestamp] + [sender duration]);
    }
}

@end
