//
//  CicadaSettingAndConfigView.h
//  CicadaPlayerDemo
//
//  Created by 郦立 on 2019/1/7.
//  Copyright © 2019年 com.alibaba. All rights reserved.
//

#import <UIKit/UIKit.h>

@class CicadaSettingAndConfigView;

@protocol CicadaSettingAndConfigViewDelegate <NSObject>

@optional

/**
 switch按钮点击回调

 @param view settingAndConfigView
 @param index 0:自动播放,1:静音,2:循环,3:硬解码,4:精准seek
 @param isOpen 是否打开
 */
- (void)settingAndConfigView:(CicadaSettingAndConfigView *)view switchChangedIndex:(NSInteger)index isOpen:(BOOL)isOpen;

/**
 声音进度条点击回调

 @param view settingAndConfigView
 @param value 进度值
 */
- (void)settingAndConfigView:(CicadaSettingAndConfigView *)view voiceSliderDidChange:(CGFloat)value;

/**
 segmented点击回调

 @param view settingAndConfigView
 @param index 0:缩放模式,1:镜像模式,2:旋转模式,3:倍速播放
 @param selectedIndex 选择了第几个seg
 */
- (void)settingAndConfigView:(CicadaSettingAndConfigView *)view segmentedControlIndex:(NSInteger)index selectedIndex:(NSInteger)selectedIndex;

/**
 底部按钮点击回调

 @param view settingAndConfigView
 @param index 0:媒体信息,1:刷新配置,2:cache刷新配置
 */
- (void)settingAndConfigView:(CicadaSettingAndConfigView *)view bottonButtonClickIndex:(NSInteger)index;

/**
 tableview点击回调

 @param view settingAndConfigView
 @param info 选择的track
 */
- (void)settingAndConfigView:(CicadaSettingAndConfigView *)view tableViewDidSelectTrack:(CicadaTrackInfo *)info;

/**
tableview点击外挂字幕回调

@param view settingAndConfigView
@param index 选择的index
@param key 选择的键
*/
- (void)settingAndConfigView:(CicadaSettingAndConfigView *)view tableViewSelectSubtitle:(int)index subtitleKey:(NSString *)key;

@end

@interface CicadaSettingAndConfigView : UIView

/**
 代理
 */
@property (nonatomic,weak) id <CicadaSettingAndConfigViewDelegate> delegate;

/**
 是否精准seek
 */
@property (nonatomic,assign,readonly)BOOL isAccurateSeek;

/**
 是否后台播放
 */
@property (nonatomic,assign,readonly)BOOL isPlayBackgournd;

/**
 外挂字幕资源
 */
@property (nonatomic,strong)NSMutableDictionary * subtitleIndexDictionary;

/**
 刷新数据

 @param array 数据
 */
- (void)setDataAndReloadWithArray:(NSMutableArray *)array;

/**
 刷新数据
 
 @param array 选中数据
 */
- (void)setSelectedDataAndReloadWithArray:(NSMutableArray *)array;

/**
 返回配置数组

 @return 配置数组
 */
- (NSArray *)getConfigArray;

/**
 返回cache配置数组
 
 @return cache配置字典
 */
- (NSDictionary *)getCacheConfigDictionary;

/**
 设置配置数组

 @param configArray 配置数组
 */
- (void)setConfigArray:(NSArray *)configArray;

/**
 重置列表数据
 */
- (void)resetTableViewData;

/**
刷新tableView
*/
- (void)reloadTableView;

/**
 设置音量
 */
- (void)setVolume:(CGFloat)vaule;

/**
 设置当前的video信息

 @param videoStr video信息
 */
- (void)setCurrentVideo:(NSString *)videoStr;

/**
 设置是否硬解码

 @param ishardwareDecoder 是否硬解码
 */
- (void)setIshardwareDecoder:(BOOL)ishardwareDecoder;

@end




