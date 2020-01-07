//
//  CicadaMacro.h
//  CicadaPlayerDemo
//
//  Created by 郦立 on 2019/1/2.
//  Copyright © 2019年 com.alibaba. All rights reserved.
//

#ifndef CicadaMacro_h
#define CicadaMacro_h

#define SCREEN_WIDTH       [UIScreen mainScreen].bounds.size.width
#define SCREEN_HEIGHT      [UIScreen mainScreen].bounds.size.height
#define WEAK_SELF          __weak typeof(self) weakSelf = self;
#define IS_PORTRAIT        (SCREEN_HEIGHT > SCREEN_WIDTH)
#define IS_IPHONEX         (SCREEN_WIDTH == 812 || SCREEN_WIDTH == 896 || SCREEN_HEIGHT == 812 || SCREEN_HEIGHT == 896)
#define SAFE_TOP           (IS_IPHONEX ? 44 : 20)
#define SAFE_BOTTOM        (IS_IPHONEX ? 34 : 0)
#define NAVIGATION_HEIGHT  (IS_IPHONEX ? 88 : 64)

#endif /* CicadaMacro_h */








