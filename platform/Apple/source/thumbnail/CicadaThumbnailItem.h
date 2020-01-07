#import <Foundation/Foundation.h>

/**
 @brief AliVcPlayerVideoThumbnailInfo为缩略图信息描述
 */
@interface CicadaThumbnailItem : NSObject

/**
 @brief thumbStart缩截图所表示时间区域的开始时间，单位：ms
 */
@property (nonatomic, assign) int64_t thumbStart;

/**
 @brief thumbPath缩截图所表示时间区域的结束时间，单位：ms
 */
@property (nonatomic, assign) int64_t thumbUntil;

/**
 @brief thumbPath图片路径
 */
@property (nonatomic, copy  ) NSString *thumbPath;

/**
 @brief thumbLeft左边位置
 */
@property (nonatomic, assign) int thumbLeft;

/**
 @brief thumbLeft上边位置
 */
@property (nonatomic, assign) int thumbTop;

/**
 @brief thumbWidth宽度
 */
@property (nonatomic, assign) int thumbWidth;

/**
 @brief thumbHeight高度
 */
@property (nonatomic, assign) int thumbHeight;

@end
