
#import "CicadaDelegate.h"

@interface CicadaThumbnail : NSObject {

}

@property (nullable, nonatomic, weak) id <CicadaDelegate> delegate;

- (instancetype _Nonnull )init:(NSURL *_Nonnull)URL;

- (void)getThumbnail:(long long)position;

@end
