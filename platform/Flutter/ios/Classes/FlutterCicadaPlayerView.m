//
//  FlutterAliPlayer.m
//  flutter_cicadaplayer
//
//  Created by aliyun on 2020/9/24.
//
#import "FlutterCicadaPlayerView.h"

@interface FlutterCicadaPlayerView ()

@end

@implementation FlutterCicadaPlayerView {
    UIView *_videoView;
    int64_t _viewId;
}

#pragma mark - life cycle

- (instancetype)initWithWithFrame:(CGRect)frame
                   viewIdentifier:(int64_t)viewId
                        arguments:(id _Nullable)args
                  binaryMessenger:(NSObject<FlutterBinaryMessenger> *)messenger
{
    if ([super init]) {
        _viewId = viewId;
        _videoView = [UIView new];
        NSDictionary *dic = args;
        CGFloat x = [dic[@"x"] floatValue];
        CGFloat y = [dic[@"y"] floatValue];
        CGFloat width = [dic[@"width"] floatValue];
        CGFloat height = [dic[@"height"] floatValue];
        _videoView.frame = CGRectMake(x, y, width, height);
    }
    return self;
}

- (nonnull UIView *)view
{
    return _videoView;
}

@end
