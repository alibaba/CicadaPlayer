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
        
        void clear();

        int show(int64_t index,const std::string &data);

        int hide(int64_t index,const std::string &data);

        int intHeader(const char *header);

    private:
        void *renderHandle{nullptr};
        AssHeader mHeader;
    };
}// namespace Cicada

@interface AppleCATextLayerRenderImpl : NSObject

@property(nonatomic, assign) Cicada::AssHeader mHeader;
@property(nonatomic, strong) CALayer *mLayer;
@property(nonatomic,strong)  NSMutableArray *dialogueArr;

- (void)setup:(CALayer *)view;
-(void)clear;

- (void)showDialogue:(Cicada::AssDialogue)ret atIndex:(NSInteger)index;
- (void)hideDialogue:(Cicada::AssDialogue)ret atIndex:(NSInteger)index;

@end

@interface DialogueObj : NSObject
@property(nonatomic, assign) Cicada::AssDialogue dialogue;
@property(nonatomic, assign) NSInteger index;
@property(nonatomic,strong) CATextLayer *layer;
@end

#endif//CICADAMEDIA_APPLECATEXTLAYERRENDER_H
