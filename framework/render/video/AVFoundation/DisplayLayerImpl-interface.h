//
// Created by pingkai on 2020/11/27.
//

#ifndef CICADAMEDIA_DISPLAYLAYERIMPL_H
#define CICADAMEDIA_DISPLAYLAYERIMPL_H

#import <base/media/IAFPacket.h>
class DisplayLayerImpl {
public:
    DisplayLayerImpl();
    ~DisplayLayerImpl();
    void init();
    int createLayer();
    void setDisplay(void *display);
    int renderFrame(std::unique_ptr<IAFFrame> &frame);

private:
    void *self;
};

#endif//CICADAMEDIA_DISPLAYLAYERIMPL_H
