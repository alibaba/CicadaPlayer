//
// Created by pingkai on 2021/7/15.
//

#ifndef CICADAMEDIA_APPLECATEXTLAYERRENDER_H
#define CICADAMEDIA_APPLECATEXTLAYERRENDER_H

#include <Foundation/Foundation.h>
#include <utils/AssUtils.h>

#if TARGET_OS_OSX
#import <AppKit/AppKit.h>
#import <Quartz/Quartz.h>
#define CicadaView NSView
#define CicadaImage NSImage
#define CicadaColor NSColor
#define CicadaFont NSFont
#elif TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#define CicadaView UIView
#define CicadaImage UIImage
#define CicadaColor UIColor
#define CicadaFont UIFont
#endif// TARGET_OS_OSX

namespace Cicada {
    class AppleCATextLayerRender {
    public:
        AppleCATextLayerRender();
        ~AppleCATextLayerRender();

        void setView(void *view);

        int show(const std::string &data);

        int hide(const std::string &data);

        int intHeader(const char *header);

    private:
        void *renderHandle{nullptr};
        AssHeader mHeader;
    };
}// namespace Cicada

@interface AppleCATextLayerRenderImpl : NSObject {
    NSMutableDictionary *layerDic;
    NSMutableDictionary *dialogueDic;
}

@property(nonatomic, assign) Cicada::AssHeader mHeader;
@property(nonatomic, strong) CALayer *mLayer;

- (void)setup:(CALayer *)view;

- (void)showDialogue:(Cicada::AssDialogue)ret;
- (void)hideDialogue:(Cicada::AssDialogue)ret;

@end

@interface DialogueObj : NSObject
@property(nonatomic, assign) Cicada::AssDialogue dialogue;
@end

#endif//CICADAMEDIA_APPLECATEXTLAYERRENDER_H
