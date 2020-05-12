
#import <Foundation/Foundation.h>

@protocol CicadaRenderDelegate <NSObject>
@optional

/**
 * 视频硬解渲染帧回调
 * @param pixelBuffer   渲染帧
 * @param pts   渲染帧pts
 * @return 返回YES则SDK不再显示(暂不支持)；返回NO则SDK渲染模块继续渲染
 */
- (BOOL)onVideoPixelBuffer:(CVPixelBufferRef)pixelBuffer pts:(int64_t)pts;

/**
 * 视频软解渲染帧回调
 * @param pixelBuffer   渲染帧
 * @param pts   渲染帧pts
 * @return 返回YES则SDK不再显示(暂不支持)；返回NO则SDK渲染模块继续渲染
 */
- (BOOL)onVideoRawBuffer:(uint8_t **)buffer lineSize:(int32_t *)lineSize pts:(int64_t)pts width:(int32_t)width height:(int32_t)height;

@end
