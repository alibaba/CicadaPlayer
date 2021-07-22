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

#if TARGET_OS_OSX
        void setView(NSView *view);
#elif TARGET_OS_IPHONE
        void setView(UIView *view);
#endif

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
@property(nonatomic, strong) CicadaView *mView;

- (void)setup:(CicadaView *)view;

- (void)showDialogue:(Cicada::AssDialogue)ret;
- (void)hideDialogue:(Cicada::AssDialogue)ret;

- (void)buildAssStyle:(CATextLayer *)textLayer withAssDialogue:(Cicada::AssDialogue)ret;
- (NSArray *)matchString:(NSString *)string withRegx:(NSString *)regexStr;
- (NSAttributedString *)buildAssStyleStr:(NSString *)style text:(NSString *)text defaultstyle:(Cicada::AssStyle)defaultstyle factor:(float)factor;

@end

@interface DialogueObj : NSObject
@property(nonatomic, assign) Cicada::AssDialogue dialogue;
@end

#endif//CICADAMEDIA_APPLECATEXTLAYERRENDER_H
