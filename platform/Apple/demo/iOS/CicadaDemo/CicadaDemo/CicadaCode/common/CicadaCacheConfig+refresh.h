//
//  CicadaCacheConfig+refresh.h
//  CicadaPlayerDemo
//
//  Created by 郦立 on 2019/6/5.
//  Copyright © 2019 com.alibaba. All rights reserved.
//

#import <CicadaPlayerSDK/CicadaPlayer.h>

@interface CicadaCacheConfig (refresh)

/**
 刷新Config配置
 
 @param dictionary 配置字典
 @return 当前对象
 */
- (instancetype)refreshConfigWithDictionary:(NSDictionary *)dictionary;

@end

