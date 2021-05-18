
#import <Foundation/Foundation.h>

@class CicadaFrameInfo;

@protocol CicadaRenderingDelegate <NSObject>
@optional

- (BOOL)onRenderingFrame:(CicadaFrameInfo*) frameInfo;

@end
