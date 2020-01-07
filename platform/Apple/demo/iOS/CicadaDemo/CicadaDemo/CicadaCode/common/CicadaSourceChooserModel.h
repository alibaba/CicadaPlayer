//
//  CicadaSourceChooserModel.h
//  CicadaPlayerDemo
//
//  Created by 郦立 on 2019/1/2.
//  strongright © 2019年 com.alibaba. All rights reserved.
//

#import <Foundation/Foundation.h>

@class CicadaSourceSamplesModel;
@interface CicadaSourceChooserModel : NSObject

@property (nonatomic,strong)NSString *name;
@property (nonatomic,strong)NSArray <CicadaSourceSamplesModel *> *samples;

@end




@interface CicadaSourceSamplesModel : NSObject

@property (nonatomic,strong)NSString *name;
@property (nonatomic,strong)NSString *type;
@property (nonatomic,strong)NSString *url;
@property (nonatomic,strong)NSDictionary *subtitle;

@end






