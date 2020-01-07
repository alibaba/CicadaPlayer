//
//  AliPlayerViewController.h
//  AliPlayerDemo
//
//  Created by 汪宁 on 2019/1/28.
//  Copyright © 2019年 com.alibaba. All rights reserved.
//

#import "AVPBaseViewController.h"

NS_ASSUME_NONNULL_BEGIN

@interface AliPlayerViewController : AVPBaseViewController

/**
 URLSource播放源
 */
@property (nonatomic,strong)CicadaUrlSource *urlSource;

/**
 是否刷新stsToken
 */
@property (nonatomic,assign)BOOL refreshStsToken;


@end

NS_ASSUME_NONNULL_END

