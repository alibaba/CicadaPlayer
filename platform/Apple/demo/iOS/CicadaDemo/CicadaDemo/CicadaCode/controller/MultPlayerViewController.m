//
//  MultPlayerViewController.m
//  CicadaPlayerDemo
//
//  Created by 郦立 on 2019/6/6.
//  Copyright © 2019 com.alibaba. All rights reserved.
//

#import "MultPlayerViewController.h"

@interface MultPlayerViewCell : UIView

@property (nonatomic,strong)CicadaPlayer *player;
@property (nonatomic,strong)NSString *playUrl;
@property (nonatomic,strong)UIView *playView;
@property (nonatomic,assign)NSInteger cellIndex;

@end

@implementation MultPlayerViewCell : UIView

- (instancetype)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        self.backgroundColor = [UIColor blackColor];
        
        self.playView = [[UIView alloc]initWithFrame:CGRectMake(5, 5, self.frame.size.width-10, self.frame.size.height-50)];
        self.playView.backgroundColor = [UIColor lightGrayColor];
        [self addSubview:self.playView];
        
        CicadaPlayer *player = [[CicadaPlayer alloc]init];
        player.autoPlay = YES;
        player.playerView = self.playView;
        self.player = player;
        
        NSArray *btnTitleArray = @[NSLocalizedString(@"准备" , nil),NSLocalizedString(@"开始" , nil),NSLocalizedString(@"暂停" , nil),NSLocalizedString(@"停止" , nil)];
        CGFloat btnWidth = (self.frame.size.width - 25)/4;
        for (int i = 0; i<btnTitleArray.count; i++) {
            UIButton *button = [[UIButton alloc]initWithFrame:CGRectMake((5+btnWidth)*i+5, self.frame.size.height-40, btnWidth, 40)];
            button.backgroundColor = [UIColor darkGrayColor];
            button.tag = i;
            [button setTitle:btnTitleArray[i] forState:UIControlStateNormal];
            [button addTarget:self action:@selector(btnClickFunc:) forControlEvents:UIControlEventTouchUpInside];
            [self addSubview:button];
        }
    }
    return self;
}

- (void)btnClickFunc:(UIButton *)sender {
    switch (sender.tag) {
        case 0: {
            CicadaUrlSource *source = [[CicadaUrlSource alloc] urlWithString:self.playUrl];
            [self.player setUrlSource:source];
            [self.player prepare];
        }
            break;
        case 1: {
            [self.player start];
        }
            break;
        case 2: {
            [self.player pause];
        }
            break;
        case 3: {
            [self.player stop];
        }
            break;
        default:
            break;
    }
}

@end



@interface MultPlayerViewController ()

@property (nonatomic,strong)UIScrollView *scrollView;
@property (nonatomic,strong)NSMutableArray *viewCellsArray;

@end

@implementation MultPlayerViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    self.title = NSLocalizedString(@"多实例播放" , nil);
    
    self.scrollView = [[UIScrollView alloc]initWithFrame:CGRectMake(0, NAVIGATION_HEIGHT, self.view.frame.size.width, self.view.frame.size.height - NAVIGATION_HEIGHT-SAFE_BOTTOM)];
    [self.view addSubview:self.scrollView];
    self.viewCellsArray = [NSMutableArray array];
    
    NSInteger cellHei = (self.view.frame.size.width-10)*9/16+50;
    for (int i = 0; i<self.playSourceArray.count; i++) {
        MultPlayerViewCell *cell = [[MultPlayerViewCell alloc]initWithFrame:CGRectMake(0, cellHei * i, self.view.frame.size.width, cellHei)];
        cell.cellIndex = i;
        cell.playUrl = self.playSourceArray[i];
        [self.scrollView addSubview:cell];
        [self.viewCellsArray addObject:cell];
    }
    self.scrollView.contentSize = CGSizeMake(self.view.frame.size.width, cellHei*self.playSourceArray.count);
}

- (void)viewDidDisappear:(BOOL)animated {
    [super viewDidDisappear:animated];
    
    for (MultPlayerViewCell *cell in self.viewCellsArray) {
        [cell.player stop];
        [cell.player destroy];
    }
}

@end

