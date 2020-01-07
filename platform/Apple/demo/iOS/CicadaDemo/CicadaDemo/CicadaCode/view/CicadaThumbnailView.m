//
//  CicadaThumbnailView.m
//  CicadaPlayerDemo
//
//  Created by 郦立 on 2019/7/25.
//  Copyright © 2019 com.alibaba. All rights reserved.
//

#import "CicadaThumbnailView.h"

@interface CicadaThumbnailView()

@property (nonatomic, strong)UILabel *timeLabel;
@property (nonatomic, strong)UIImageView *thumbnailImageView;

@end

@implementation CicadaThumbnailView

- (instancetype)init {
    if (self = [super init]) {
        
        self.backgroundColor = [UIColor clearColor];
        
        self.timeLabel = [[UILabel alloc]init];
        self.timeLabel.textAlignment = NSTextAlignmentCenter;
        self.timeLabel.textColor = [UIColor whiteColor];
        self.timeLabel.font = [UIFont systemFontOfSize:20];
        [self addSubview:self.timeLabel];
        
        self.thumbnailImageView = [[UIImageView alloc]init];
        self.thumbnailImageView.contentMode = UIViewContentModeScaleAspectFit;
        self.thumbnailImageView.layer.masksToBounds = YES;
        [self addSubview:self.thumbnailImageView];
        
        self.hidden = YES;
    }
    return self;
}

- (void)layoutSubviews {
    [super layoutSubviews];
    CGFloat width = self.frame.size.width;
    CGFloat height = self.frame.size.height;
    self.timeLabel.frame = CGRectMake(0, 0, width, 30);
    self.thumbnailImageView.frame = CGRectMake(0, 30, width, height - 30);
}

- (void)showThumbnailWithImage:(UIImage *)image title:(NSString *)title {
    self.thumbnailImageView.image = image;
    self.timeLabel.text = title;
    self.hidden = NO;
}

@end
