//
// Created by pingkai on 2020/9/8.
//

#include "appleLog.h"
#import <Foundation/Foundation.h>

void appleNSlogC(const char level, const char *fmt)
{
    NSLog(@"AliFrameWork %c %s", level, fmt);
}