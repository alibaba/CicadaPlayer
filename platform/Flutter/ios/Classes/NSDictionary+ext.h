//
//  NSDictionary+ext.h
//  flutter_cicadaplayer
//
//  Created by aliyun on 2020/11/27.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface NSDictionary (ext)

- (NSDictionary *)removeNull;

- (id)getStrByKey:(NSString *)key;

@end

NS_ASSUME_NONNULL_END
