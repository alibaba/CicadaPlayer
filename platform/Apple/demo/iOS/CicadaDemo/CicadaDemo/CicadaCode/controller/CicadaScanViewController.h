//
//  CicadaScanViewController.h
//  CicadaDemo
//
//  Created by ToT on 2019/12/19.
//  Copyright © 2019 com.alibaba. All rights reserved.
//

#import "CicadaBaseViewController.h"

typedef void(^scanCallBack)(NSString *text);

@interface CicadaScanViewController : CicadaBaseViewController

@property (nonatomic,strong)scanCallBack scanTextCallBack;

@end


