//
//  CicadaPlayerGlobalSettings.h
//  CicadaPlayerGlobalSettings
//
//  Created by huang_jiafa on 2020/04/09.
//  Copyright © 2020 com.alibaba.AliyunPlayer. All rights reserved.
//

#import <Foundation/Foundation.h>

OBJC_EXPORT
@interface CicadaPlayerGlobalSettings : NSObject

/**
 @brief 设置域名对应的解析ip
 @param host 域名，需指定端口（http默认端口80，https默认端口443）。例如player.alicdn.com:443
 @param ip 相应的ip，设置为空字符串清空设定。
 */
/****
 @brief Set a DNS ip to resolve the host.
 @param host The host. Need to specify the port (http defualt port is 80，https default port is 443). E.g player.alicdn.com:443
 @param ip The ip address, set as empty string to clear the setting.
 */
+(void)setDNSResolve:(NSString *)host ip:(NSString *)ip;

/**
 @brief 设置是否使能硬件提供的音频变速播放能力，关闭后则使用软件实现音频的倍速播放，pcm回调数据的格式和此设置关联,如果修改，请在同一个线程操作,默认打开
 */
/****
 @brief enable/disable hardware audio tempo, player will use soft ware tempo filter when disabled, and it will affect the pcm data that from audio rending callback, it only can be reset in the same thread, enable by default.
 */
+ (void)enableHWAduioTempo:(bool)enable;

@end
