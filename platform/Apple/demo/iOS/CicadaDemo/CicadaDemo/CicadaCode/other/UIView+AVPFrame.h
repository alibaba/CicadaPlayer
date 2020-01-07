//
//  UIView+AVPFrame.h
//  CicadaPlayerDemo
//
//  Created by 郦立 on 2019/1/7.
//  Copyright © 2019年 com.alibaba. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface UIView (AVPFrame)

/**
 * 获取视图 x 坐标
 *
 *  @return x 坐标
 */
- (CGFloat)getX;

/**
 * 获取视图 y 坐标
 *
 *  @return y 坐标
 */
- (CGFloat)getY;

/**
 * 获取视图 宽度
 *
 *  @return 宽度
 */
- (CGFloat)getWidth;

/**
 * 获取视图 高度
 *
 *  @return 高度
 */
- (CGFloat)getHeight;

/**
 * 获取视图 最右边距屏幕左边的距离
 *
 *  @return 距离
 */
- (CGFloat)getMaxX;

/**
 * 获取视图 最下边距屏幕左边的距离
 *
 *  @return 距离
 */
- (CGFloat)getMaxY;

/**
 * 获取视图 最左边 距 父视图左边 距离
 *
 *  @return 距离
 */
- (CGFloat)getMinX;

/**
 * 获取视图 最上边 距 父视图上边 距离
 *
 *  @return 距离
 */
- (CGFloat)getMinY;

/**
 *  获取视图 中心点 X
 *
 *  @return center.x
 */
- (CGFloat)getMidX;

/**
 *  获取视图 中心点 Y
 *
 *  @return center.y
 */
- (CGFloat)getMidY;

/**
 设置origin
 
 @param origin origin
 */
- (void)setOrigin:(CGPoint)origin;

/**
 设置size
 
 @param size size
 */
- (void)setSize:(CGSize)size ;

@end
