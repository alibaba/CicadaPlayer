#import <Foundation/Foundation.h>

@interface CicadaURLSession : NSObject <NSURLSessionDelegate> {

}

- (instancetype _Nonnull )init;
- (void) request:(NSURL *_Nonnull)URL
        complete:(void (^_Nullable)(CicadaURLSession * _Nullable session, NSData * _Nullable data))complete;
- (void) cancel;

@end
