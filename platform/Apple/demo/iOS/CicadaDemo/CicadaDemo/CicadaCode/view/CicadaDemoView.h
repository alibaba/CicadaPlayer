//
//  CicadaDemoView.h
//  CicadaPlayerDemo
//
//  Created by 郦立 on 2019/1/7.
//  Copyright © 2019年 com.alibaba. All rights reserved.
//

#import <UIKit/UIKit.h>

@class CicadaDemoView;

@protocol CicadaDemoViewDelegate <NSObject>

@optional

/**
 底部按钮点击回调

 @param playerView playerView
 @param index 0:准备,1:播放,2:暂停,3:停止,4:截图,5:重试
 */
- (void)CicadaDemoView:(CicadaDemoView *)playerView bottonButtonClickAtIndex:(NSInteger)index;

/**
 全屏按钮点击回调

 @param playerView playerView
 @param isFull 是否全屏
 */
- (void)CicadaDemoView:(CicadaDemoView *)playerView fullScreenButtonAction:(BOOL)isFull;

/**
 进度条完成进度回调

 @param playerView playerView
 @param value 进度值
 */
- (void)CicadaDemoView:(CicadaDemoView *)playerView progressSliderDidChange:(CGFloat)value;

/**
 进度条改变进度回调
 
 @param playerView playerView
 @param value 进度值
 */
- (void)CicadaDemoView:(CicadaDemoView *)playerView progressSliderValueChanged:(CGFloat)value;

@end

@interface CicadaDemoView : UIView

/**
 代理
 */
@property (nonatomic,weak) id <CicadaDemoViewDelegate> delegate;

/**
 播放视图
 */
@property (nonatomic,strong)UIView *playerView;

/**
 缓存位置
 */
@property (nonatomic,assign)int64_t bufferPosition;

/**
 当前播放位置
 */
@property (nonatomic,assign)int64_t currentPosition;

/**
 视频总时长
 */
@property (nonatomic,assign)int64_t allPosition;

/**
 标题label
 */
@property (nonatomic,strong)UILabel *subTitleLabel;

/**
 是否隐藏进度条
 */
@property (nonatomic,assign)BOOL hiddenSlider;

/**
 展示缩略图

 @param image 缩略图图片
 */
- (void)showThumbnailViewWithImage:(UIImage *)image;

/**
 隐藏缩略图
 */
- (void)hiddenThumbnailView;

/**
 隐藏加载视图
*/
- (void)hiddenLoadingView;

/**
 显示加载视图
*/
- (void)showLoadingView;

/**
 设置加载进度
*/
- (void)setLoadingViewProgress:(int)progress;

/**
 设置文字并且显示
*/
- (void)setSubtitleAndShow:(NSString *)subtitle;

@end






