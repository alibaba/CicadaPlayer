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

- (int)createLayer;

- (void)setDisplay:(void *)layer;

@end


#endif//CICADAMEDIA_SAMPLEDISPLAYLAYERRENDER_H
