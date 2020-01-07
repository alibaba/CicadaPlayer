//
// Created by moqi on 2019-09-05.
//
#import <Foundation/Foundation.h>
//#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#include "CADisplayLinkVSync.h"
#import "CAApplicationCaller.h"

CADisplayLinkVSync::CADisplayLinkVSync(IVSync::Listener &listener) : IVSync(listener)
{
    c = [[CAApplicationCaller alloc] init:&listener];
}

CADisplayLinkVSync::~CADisplayLinkVSync()
{
    [(id)c release];
}

void CADisplayLinkVSync::start()
{
    [(id)c startMainLoop];
}

void CADisplayLinkVSync::pause()
{

}
