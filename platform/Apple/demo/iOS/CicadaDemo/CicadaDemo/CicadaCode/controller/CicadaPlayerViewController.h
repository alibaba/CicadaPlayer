//
//  CicadaPlayerViewController.h
//  CicadaPlayerDemo
//
//  Created by 郦立 on 2019/1/2.
//  Copyright © 2019年 com.alibaba. All rights reserved.
//

#import "CicadaBaseViewController.h"

@interface CicadaPlayerViewController : CicadaBaseViewController

/**
 URLSource播放源
 */
@property (nonatomic,strong)CicadaUrlSource *urlSource;

/**
 是否刷新stsToken
 */
@property (nonatomic,assign)BOOL refreshStsToken;

/**
 外挂字幕资源
 */
@property (nonatomic,strong)NSDictionary * subtitleDictionary;

/**
fairPlay标识，如果使用了FairPlay，avResourceLoaderDelegate也要同步设置
*/
@property (nonatomic, assign) BOOL useFairPlay;
/**
ResourceLoaderDelegate
*/
@property (nonatomic,strong) NSObject<AVAssetResourceLoaderDelegate>* avResourceLoaderDelegate;

@end








