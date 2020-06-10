#ifndef CicadaAudioSessionDelegate_h
#define CicadaAudioSessionDelegate_h

#import <AVFoundation/AVFoundation.h>
#import <TargetConditionals.h>

@protocol CicadaAudioSessionDelegate <NSObject>
#if TARGET_OS_IPHONE
@optional
- (BOOL)setActive:(BOOL)active error:(NSError **)outError;

@optional
- (BOOL)setCategory:(NSString *)category withOptions:(AVAudioSessionCategoryOptions)options error:(NSError **)outError;

@optional
- (BOOL)setCategory:(AVAudioSessionCategory)category mode:(AVAudioSessionMode)mode routeSharingPolicy:(AVAudioSessionRouteSharingPolicy)policy options:(AVAudioSessionCategoryOptions)options error:(NSError **)outError;
#endif
@end

#endif /* CicadaAudioSessionDelegate_h */
