//
//  CicadaDemoView.m
//  CicadaPlayerDemo
//
//  Created by 郦立 on 2019/1/7.
//  Copyright © 2019年 com.alibaba. All rights reserved.
//

#import "CicadaDemoView.h"
#import "CicadaThumbnailView.h"

@interface CicadaDemoView()

@property (nonatomic,strong)UILabel *timeLabel;
@property (nonatomic,strong)UILabel *bufferLabel;
@property (nonatomic,strong)NSString *currentPositionString;
@property (nonatomic,strong)NSString *allPositionString;
@property (nonatomic,strong)UIProgressView *progressView;
@property (nonatomic,strong)UISlider *progressSlider;
@property (nonatomic,strong)NSMutableArray *buttonsArray;
@property (nonatomic,strong)UIButton *fullScreenButton;
@property (nonatomic,assign)CGRect myFrame;
@property (nonatomic,assign)BOOL sliderIsTouchDown;
@property (nonatomic,strong)CicadaThumbnailView *thumbnailView;
@property (nonatomic,strong)UIActivityIndicatorView *loadingView;
@property (nonatomic,strong)UILabel *loadingProgressLabel;

@end

@implementation CicadaDemoView

- (instancetype)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        
        self.myFrame = frame;
        self.buttonsArray = [NSMutableArray array];
        self.backgroundColor = [UIColor whiteColor];
        self.currentPositionString = @"00:00:00";
        self.allPositionString = @"00:00:00";
        
        CGFloat selfWidth = self.frame.size.width;
        CGFloat selfHeight = self.frame.size.height;
        
        self.playerView = [[UIView alloc]init];
        self.playerView.backgroundColor = [UIColor blackColor];
        [self addSubview:self.playerView];
        
        self.subTitleLabel = [[UILabel alloc]init];
        self.subTitleLabel.font = [UIFont systemFontOfSize:15];
        self.subTitleLabel.textColor = [UIColor whiteColor];
        self.subTitleLabel.textAlignment = NSTextAlignmentCenter;
        self.subTitleLabel.numberOfLines = 0;
        self.subTitleLabel.backgroundColor = [UIColor colorWithWhite:0.5 alpha:0.5];
        self.subTitleLabel.layer.masksToBounds = YES;
        self.subTitleLabel.layer.cornerRadius = 3;
        [self addSubview:self.subTitleLabel];
        self.subTitleLabel.hidden = YES;
        
        self.timeLabel = [[UILabel alloc]init];
        self.timeLabel.font = [UIFont systemFontOfSize:14];
        self.timeLabel.textColor = [UIColor whiteColor];
        [self setTimeLabelText];
        [self addSubview:self.timeLabel];
        
        self.bufferLabel = [[UILabel alloc]init];
        self.bufferLabel.font = [UIFont systemFontOfSize:14];
        self.bufferLabel.textColor = [UIColor whiteColor];
        self.bufferLabel.text = @"buffer: 00:00:00";
        [self addSubview:self.bufferLabel];
        
        self.fullScreenButton = [[UIButton alloc]init];
        [self.fullScreenButton setTitle:NSLocalizedString(@"全屏" , nil) forState:UIControlStateNormal];
        self.fullScreenButton.titleLabel.font = [UIFont systemFontOfSize:14];
        [self.fullScreenButton setTitleColor:[UIColor colorWithRed:54/255.0 green:126/255.0 blue:251/255.0 alpha:1] forState:UIControlStateNormal];
        [self.fullScreenButton setTitleColor:[UIColor lightGrayColor] forState:UIControlStateHighlighted];
        [self.fullScreenButton addTarget:self action:@selector(fullScreenButtonClick:) forControlEvents:UIControlEventTouchUpInside];
        [self addSubview:self.fullScreenButton];
        
        self.progressView = [[UIProgressView alloc] init];
        self.progressView.trackTintColor = [UIColor whiteColor];
        self.progressView.progressTintColor = [UIColor grayColor];
        [self addSubview:self.progressView];
        
        self.progressSlider = [[UISlider alloc]init];
        self.progressSlider.maximumTrackTintColor = [UIColor clearColor];
        [self.progressSlider addTarget:self action:@selector(progressSliderTouchDown:) forControlEvents:UIControlEventTouchDown];
        [self.progressSlider addTarget:self action:@selector(progressSliderEventValueChanged:) forControlEvents:UIControlEventValueChanged];
        [self.progressSlider addTarget:self action:@selector(progressSliderDidChange:) forControlEvents:UIControlEventTouchUpInside];
        [self.progressSlider addTarget:self action:@selector(progressSliderDidChange:) forControlEvents:UIControlEventTouchUpOutside];
        [self addSubview:self.progressSlider];
        
        NSArray *buttonTitleArray = @[NSLocalizedString(@"准备" , nil),NSLocalizedString(@"播放" , nil),NSLocalizedString(@"暂停" , nil),NSLocalizedString(@"停止" , nil),NSLocalizedString(@"截图" , nil)];
        NSInteger arrayCount = buttonTitleArray.count;
        for (int i = 0; i<arrayCount; i++) {
            UIButton *button = [[UIButton alloc]initWithFrame:CGRectMake(selfWidth/arrayCount*i, selfHeight-44, selfWidth/arrayCount, 44)];
            [button setTitle:buttonTitleArray[i] forState:UIControlStateNormal];
            button.titleLabel.font = [UIFont systemFontOfSize:14];
            [button setTitleColor:[UIColor colorWithRed:54/255.0 green:126/255.0 blue:251/255.0 alpha:1] forState:UIControlStateNormal];
            [button setTitleColor:[UIColor lightGrayColor] forState:UIControlStateHighlighted];
            [button addTarget:self action:@selector(bottonButtonClick:) forControlEvents:UIControlEventTouchUpInside];
            button.tag = i;
            if (@available(iOS 13.0, *)) {
                button.backgroundColor = [UIColor systemBackgroundColor];
            }
            [self.buttonsArray addObject:button];
            [self addSubview:button];
        }
        
        self.thumbnailView = [[CicadaThumbnailView alloc]init];
        [self addSubview:self.thumbnailView];
        
        self.loadingView = [[UIActivityIndicatorView alloc]initWithFrame:CGRectMake(0, 0, 30, 30)];
        self.loadingView.hidden = YES;
        [self addSubview:self.loadingView];
        self.loadingProgressLabel = [[UILabel alloc]initWithFrame:CGRectMake(0, 0, 200, 30)];
        self.loadingProgressLabel.textColor = [UIColor whiteColor];
        self.loadingProgressLabel.font = [UIFont systemFontOfSize:14];
        self.loadingProgressLabel.textAlignment = NSTextAlignmentCenter;
        self.loadingProgressLabel.hidden = YES;
        [self addSubview:self.loadingProgressLabel];
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(layoutSubviews) name:UIDeviceOrientationDidChangeNotification object:nil];
    }
    return self;
}

- (void)bottonButtonClick:(UIButton *)sender {
    if ([self.delegate respondsToSelector:@selector(CicadaDemoView:bottonButtonClickAtIndex:)]) {
        [self.delegate CicadaDemoView:self bottonButtonClickAtIndex:sender.tag];
    }
}

- (void)fullScreenButtonClick:(UIButton *)sender {
    if ([self.delegate respondsToSelector:@selector(CicadaDemoView:fullScreenButtonAction:)]) {
        [self.delegate CicadaDemoView:self fullScreenButtonAction:YES];
    }
    
    NSNumber *value = [NSNumber numberWithInt:UIInterfaceOrientationLandscapeRight];
    if (!IS_PORTRAIT) {
        value = [NSNumber numberWithInt:UIInterfaceOrientationPortrait];
    }
    [[UIDevice currentDevice] setValue:value forKey:@"orientation"];
}

- (void)layoutSubviews {
    [super layoutSubviews];
    
    BOOL hideButton = YES;
    if (IS_PORTRAIT) {
        UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
        bool iphonexLeft = (orientation == UIDeviceOrientationLandscapeLeft || orientation == UIDeviceOrientationLandscapeRight);
        if (IS_IPHONEX && iphonexLeft) {
            NSNumber *value = [NSNumber numberWithInt:UIInterfaceOrientationPortrait];
            [[UIDevice currentDevice] setValue:value forKey:@"orientation"];
            return;
        }else {
            self.frame = self.myFrame;
            hideButton = NO;
            [self.fullScreenButton setTitle:NSLocalizedString(@"全屏" , nil) forState:UIControlStateNormal];
        }
    }else {
        self.frame = [UIScreen mainScreen].bounds;
        [self.fullScreenButton setTitle:NSLocalizedString(@"半屏" , nil) forState:UIControlStateNormal];
    }
    for (UIButton *btn in self.buttonsArray) {
        btn.hidden = hideButton;
    }
    [self initViews];
}


- (void)initViews {
    CGFloat selfWidth = self.frame.size.width;
    CGFloat selfHeight = self.frame.size.height;
    NSInteger buttonHeight = 44;
    NSInteger bottonEdge = 0;
    if (IS_PORTRAIT) {
        bottonEdge = 44;
    }
    self.subTitleLabel.frame = CGRectMake((self.frame.size.width-self.subTitleLabel.frame.size.width)/2, IS_PORTRAIT?20:64, self.subTitleLabel.frame.size.width, self.subTitleLabel.frame.size.height);
    self.playerView.frame = CGRectMake(0, 0, selfWidth, selfHeight-bottonEdge);
    self.timeLabel.frame = CGRectMake(15, selfHeight-buttonHeight-bottonEdge, 150, buttonHeight);
    self.bufferLabel.frame = CGRectMake(15, selfHeight-76-bottonEdge, 135, buttonHeight);
    self.fullScreenButton.frame = CGRectMake(selfWidth-60, selfHeight-buttonHeight-bottonEdge, 60, buttonHeight);
    self.progressView.frame = CGRectMake(152, selfHeight-buttonHeight-bottonEdge+21, selfWidth-214, 2);
    self.progressSlider.frame = CGRectMake(150, selfHeight-buttonHeight-bottonEdge, selfWidth-210, buttonHeight);
    self.thumbnailView.frame = CGRectMake(selfWidth/2-80, (selfHeight-bottonEdge)/2-60, 160, 120);
    self.loadingView.center = CGPointMake(self.playerView.center.x, self.playerView.center.y - 20);
    self.loadingProgressLabel.center = CGPointMake(self.playerView.center.x, self.playerView.center.y + 10);
}

- (void)progressSliderTouchDown:(UISlider *)sender {
    self.sliderIsTouchDown = YES;
}

- (void)progressSliderDidChange:(UISlider *)sender {
    self.sliderIsTouchDown = NO;
    if ([self.delegate respondsToSelector:@selector(CicadaDemoView:progressSliderDidChange:)]) {
        [self.delegate CicadaDemoView:self progressSliderDidChange:sender.value];
    }
}

- (void)progressSliderEventValueChanged:(UISlider *)sender {
    if (self.allPosition <= 0) {
        return;
    }
    _currentPosition = self.allPosition * sender.value;
    self.currentPositionString = [self getMMSSFromSS:[NSString stringWithFormat:@"%lld",(self.currentPosition+500)/1000]];
    [self setTimeLabelText];
    
    if ([self.delegate respondsToSelector:@selector(CicadaDemoView:progressSliderValueChanged:)]) {
        [self.delegate CicadaDemoView:self progressSliderValueChanged:sender.value];
    }
}

- (void)setBufferPosition:(int64_t)bufferPosition {
    _bufferPosition = bufferPosition;
    self.bufferLabel.text = [NSString stringWithFormat:@"buffer: %@",[self getMMSSFromSS:[NSString stringWithFormat:@"%lld",(bufferPosition+500)/1000]]];
    if (self.allPosition > 0) {
        CGFloat allPositionFloat = (CGFloat)self.allPosition;
        self.progressView.progress = bufferPosition/allPositionFloat;
    }else {
        self.progressView.progress = 0;
    }
}

- (void)setCurrentPosition:(int64_t)currentPosition {
    if (self.allPosition != 0 && !self.sliderIsTouchDown) {
        _currentPosition = currentPosition;
        self.currentPositionString = [self getMMSSFromSS:[NSString stringWithFormat:@"%lld",(currentPosition+500)/1000]];
        [self setTimeLabelText];
        CGFloat allPositionFloat = (CGFloat)self.allPosition;
        self.progressSlider.value = currentPosition/allPositionFloat;
    }

}

- (void)setAllPosition:(int64_t)allPosition {
    _allPosition = allPosition;
    self.allPositionString = [self getMMSSFromSS:[NSString stringWithFormat:@"%lld",(allPosition+500)/1000]];
    [self setTimeLabelText];
}

- (void)setTimeLabelText {
    self.timeLabel.text = [NSString stringWithFormat:@"%@/%@",self.currentPositionString,self.allPositionString];
}

- (void)setHiddenSlider:(BOOL)hiddenSlider {
    _hiddenSlider = hiddenSlider;
    self.progressSlider.hidden = hiddenSlider;
}

- (void)showThumbnailViewWithImage:(UIImage *)image {
    if (self.sliderIsTouchDown) {
        [self.thumbnailView showThumbnailWithImage:image title:[self getMMSSFromSS:[NSString stringWithFormat:@"%lld",self.currentPosition/1000]]];
    }
}

- (void)hiddenThumbnailView {
    self.thumbnailView.hidden = YES;
}

- (void)hiddenLoadingView {
    [self.loadingView stopAnimating];
    self.loadingView.hidden = YES;
    self.loadingProgressLabel.hidden = YES;
}

- (void)showLoadingView {
    [self.loadingView startAnimating];
    self.loadingView.hidden = NO;
    self.loadingProgressLabel.text = @"loading ... 0%";
    self.loadingProgressLabel.hidden = NO;
}

- (void)setLoadingViewProgress:(int)progress {
    NSString *progressLabelTitle = [NSString stringWithFormat:@"loading ... %d %%",progress];
    self.loadingProgressLabel.text = progressLabelTitle;
}

- (void)setSubtitleAndShow:(NSString *)subtitle {
    CGSize subtitleSize = [self getSubTitleLabelFrameWithSubtitle:subtitle];
    self.subTitleLabel.frame = CGRectMake((self.frame.size.width-subtitleSize.width)/2, IS_PORTRAIT?20:64, subtitleSize.width, subtitleSize.height);
    self.subTitleLabel.text = subtitle;
    self.subTitleLabel.hidden = NO;
}

- (CGSize)getSubTitleLabelFrameWithSubtitle:(NSString *)subtitle {
    NSArray *subsectionArray = [subtitle componentsSeparatedByString:@"\n"];
    CGFloat maxWidth = 0;
    for (NSString *subsectionTitle in subsectionArray) {
        NSDictionary *dic = @{NSFontAttributeName:self.subTitleLabel.font};
        CGRect rect = [subsectionTitle boundingRectWithSize:CGSizeMake(9999, 18) options:NSStringDrawingUsesLineFragmentOrigin|NSStringDrawingUsesFontLeading attributes:dic context:nil];
        if (rect.size.width > maxWidth) { maxWidth = rect.size.width; }
    }
    return CGSizeMake(maxWidth + 10 , 18 * subsectionArray.count + 10 );
}

- (NSString *)getMMSSFromSS:(NSString *)totalTime{
    NSInteger seconds = [totalTime integerValue];
    //format of hour
    NSString *str_hour = [NSString stringWithFormat:@"%02ld",seconds/3600];
    //format of minute
    NSString *str_minute = [NSString stringWithFormat:@"%02ld",(seconds%3600)/60];
    //format of second
    NSString *str_second = [NSString stringWithFormat:@"%02ld",seconds%60];
    //format of time
    NSString *format_time = [NSString stringWithFormat:@"%@:%@:%@",str_hour,str_minute,str_second];
    return format_time;
}

@end






