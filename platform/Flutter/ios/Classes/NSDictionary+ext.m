//
//  NSDictionary+ext.m
//  flutter_cicadaplayer
//
//  Created by aliyun on 2020/11/27.
//

#import "NSDictionary+ext.h"

@implementation NSDictionary (ext)

- (NSDictionary *)removeNull
{
    NSArray *keyArr = [self allKeys];
    NSMutableDictionary *resDic = [[NSMutableDictionary alloc] init];
    for (int i = 0; i < keyArr.count; i++) {
        id obj = [self getStrByKey:keyArr[i]];
        [resDic setObject:obj forKey:keyArr[i]];
    }
    return resDic;
}

- (id)getStrByKey:(NSString *)key
{
    id val = [self objectForKey:key];
    if ([val isKindOfClass:[NSNull class]]) {
        val = @"";
    }
    return val;
}

@end
