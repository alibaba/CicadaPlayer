//
// Created by pingkai on 2020/11/27.
//

#ifndef CICADAMEDIA_SAMPLEDISPLAYLAYERRENDER_H
#define CICADAMEDIA_SAMPLEDISPLAYLAYERRENDER_H

#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>

@interface SampleDisplayLayerRender : NSObject {
    //  AVSampleBufferDisplayLayer* displayLayer;
}
@property(nonatomic, strong) AVSampleBufferDisplayLayer *displayLayer;

@property CATransform3D scaleTransform;
@property CATransform3D mirrorTransform;
@property CATransform3D rotateTransform;

@property(nonatomic, assign) BOOL isFillWidth;
@property(nonatomic) CGSize videoSize;

@property(nonatomic) int rotateMode;

- (int)createLayer;

- (void)setDisplay:(void *)layer;

- (void)setVideoSize:(CGSize)videoSize;
@end


#endif//CICADAMEDIA_SAMPLEDISPLAYLAYERRENDER_H
