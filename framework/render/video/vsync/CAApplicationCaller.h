
#import <Foundation/Foundation.h>
#import "IVSync.h"

@interface CAApplicationCaller : NSObject {
    id displayLink;
    int interval;
    IVSync::Listener *lisener;
}
@property (readwrite) int interval;

- (instancetype)init:(IVSync::Listener *)syncLisener;
-(void) startMainLoop;
-(void) doCaller;

@end
