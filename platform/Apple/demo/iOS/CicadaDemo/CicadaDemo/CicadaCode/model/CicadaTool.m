//
//  CicadaTool.m
//  CicadaPlayerDemo
//
//  Created by 郦立 on 2018/12/29.
//  Copyright © 2018年 com.alibaba. All rights reserved.
//

#import "CicadaTool.h"
#import "CicadaSourceChooserModel.h"
#import "MJExtension.h"
#import "MBProgressHUD.h"

#define HARDWARE_UD_KEY  @"ud_key_hardware"

@implementation CicadaTool

+ (NSArray *)getSourceSamplesArray {
    NSString *path = [[NSBundle mainBundle] pathForResource:@"source" ofType:@"json"];
    unsigned long encode = CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingUTF8);
    NSError *error;
    NSString *content = [NSString stringWithContentsOfFile:path encoding:encode error:&error];
    NSArray *array = [CicadaSourceChooserModel mj_objectArrayWithKeyValuesArray:content];
    return array;
}

+ (NSArray *)getSourceURLArray {
    NSArray *allSourceArray = [self getSourceSamplesArray];
    NSMutableArray *backArray = [NSMutableArray array];
    for (CicadaSourceChooserModel *model in allSourceArray) {
        for (CicadaSourceSamplesModel *inModel in model.samples) {
            if ([inModel.type isEqualToString:@"url"]) {
                [backArray addObject:inModel];
            }
        }
    }
    return backArray;
}

+ (NSMutableArray *)getCicadaConfigArray {
    NSString *path = [[NSBundle mainBundle] pathForResource:@"CicadaConfig" ofType:@"json"];
    unsigned long encode = CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingUTF8);
    NSError *error;
    NSString *content = [NSString stringWithContentsOfFile:path encoding:encode error:&error];
    NSData *jsonData = [content dataUsingEncoding:NSUTF8StringEncoding];
    NSDictionary *dic = [NSJSONSerialization JSONObjectWithData:jsonData
                                                        options:NSJSONReadingMutableContainers
                                                          error:&error];
    NSArray *keyArray = @[@"startBufferDuration",@"highBufferDuration",@"maxBufferDuration",@"maxDelayTime",@"networkTimeout",@"referer",@"httpProxy",@"networkRetryCount"];
    NSMutableArray *backArray = [NSMutableArray array];
    for (NSString *key in keyArray) {
        NSString *value = dic[key];
        [backArray addObject:value];
    }
    return backArray;
}

+ (NSDictionary *)getConfigDictionary {
    NSString *path = [[NSBundle mainBundle] pathForResource:@"CicadaConfig" ofType:@"json"];
    unsigned long encode = CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingUTF8);
    NSError *error;
    NSString *content = [NSString stringWithContentsOfFile:path encoding:encode error:&error];
    NSData *jsonData = [content dataUsingEncoding:NSUTF8StringEncoding];
    NSDictionary *dic = [NSJSONSerialization JSONObjectWithData:jsonData
                                                        options:NSJSONReadingMutableContainers
                                                          error:&error];
    return dic;
}

+ (void)hudWithText:(NSString *)text view:(UIView *)view {
    dispatch_async(dispatch_get_main_queue(), ^{
        NSString * hudShowText = text;
        NSEnumerator *subviewsEnum = [view.subviews reverseObjectEnumerator];
        for (UIView *subview in subviewsEnum) {
            if ([subview isKindOfClass:[MBProgressHUD class]] && subview.tag == 999) {
                MBProgressHUD *hud = (MBProgressHUD *)subview;
                if (hud.detailsLabel.text.length > 0) {
                    hud.hidden = YES;
                    [hud removeFromSuperview];
                    hudShowText = [[hudShowText stringByAppendingString:@"\n"] stringByAppendingString:hud.detailsLabel.text];
                }
            }
        }
        MBProgressHUD *hud = [MBProgressHUD showHUDAddedTo:view animated:YES];
        hud.tag = 999;
        hud.mode = MBProgressHUDModeText;
        hud.detailsLabel.text = hudShowText;
        hud.detailsLabel.numberOfLines = 5;
        hud.margin = 10.f;
        [hud setOffset:CGPointMake(0, SCREEN_HEIGHT*0.35)];
        hud.userInteractionEnabled = NO;
        hud.removeFromSuperViewOnHide = YES;
        [hud hideAnimated:YES afterDelay:3];
    });
}

+ (void)loadingHudToView:(UIView *)view {
    dispatch_async(dispatch_get_main_queue(), ^{
        MBProgressHUD *hud = [MBProgressHUD showHUDAddedTo:view animated:YES];
        hud.removeFromSuperViewOnHide = YES;
        hud.label.text = NSLocalizedString(@"加载中..." , nil);
    });
}

+ (void)hideLoadingHudForView:(UIView *)view {
    dispatch_async(dispatch_get_main_queue(), ^{
        NSEnumerator *subviewsEnum = [view.subviews reverseObjectEnumerator];
        for (UIView *subview in subviewsEnum) {
            if ([subview isKindOfClass:[MBProgressHUD class]]) {
                MBProgressHUD *hud = (MBProgressHUD *)subview;
                if ([hud.label.text isEqualToString:NSLocalizedString(@"加载中..." , nil)]) {
                    hud.hidden = YES;
                    [hud removeFromSuperview];
                }
            }
        }
    });
}

+ (NSString *)stringFromInt:(int)count {
    return [NSString stringWithFormat:@"%d",count];
}

+ (NSArray *)getDocumentMP4Array {
    NSString *documentPath = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES).firstObject;
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSArray *fileList = [fileManager contentsOfDirectoryAtPath:documentPath error:nil];
    NSMutableArray *dirArray = [[NSMutableArray alloc] init];
    NSArray *suffixArray = @[@"webm", @"mp4",@"mp3",@"flv",@"mov"];
    for (NSString *file in fileList) {
        if ([suffixArray containsObject:[[file pathExtension] lowercaseString]]) {
            NSString *filePath = [NSString stringWithFormat:@"%@/%@",documentPath,file];
            [dirArray addObject:filePath];
        }
    }
    return dirArray.copy;
}

+ (NSString *)infoStringDescribe:(CicadaMediaInfo *)info {
    if (!info) {
        return NSLocalizedString(@"信息暂缺" , nil);
    }
    NSMutableString *backString = [NSMutableString string];
    if (nil != info.tracks) {
        [backString appendString:@"tracks:"];
        [backString appendString:info.tracks.description];
    }
    if (backString.length == 0) {
        return NSLocalizedString(@"信息暂缺" , nil);
    }
    return backString.copy;
}

+ (NSTimeInterval)currentTimeInterval {
    return [[NSDate date] timeIntervalSince1970];
}

+ (void)showAlert:(NSString *)title sender:(UIViewController *)sender {
    if (title == nil) { return; }
    if (@available(iOS 9.0, *)) {
        if (sender == nil) {
            return;
        }else {
            UIAlertController *alert = [UIAlertController alertControllerWithTitle:nil message:title preferredStyle:UIAlertControllerStyleAlert];
            UIAlertAction *action = [UIAlertAction actionWithTitle:NSLocalizedString(@"确认" , nil) style:UIAlertActionStyleCancel handler:nil];
            [alert addAction:action];
            [sender presentViewController:alert animated:YES completion:nil];
        }
    }else {
        UIAlertView *alert = [[UIAlertView alloc] initWithTitle:nil message:title delegate:nil cancelButtonTitle:nil otherButtonTitles:NSLocalizedString(@"确认" , nil), nil];
        [alert show];
    }
}

+ (BOOL)isHardware {
    if ([[NSUserDefaults standardUserDefaults] objectForKey:HARDWARE_UD_KEY]) {
        return NO;
    }
    return YES;
}

+ (void)saveIsHardware:(BOOL)isHardware {
    if (isHardware) {
        [[NSUserDefaults standardUserDefaults] removeObjectForKey:HARDWARE_UD_KEY];
    }else {
        [[NSUserDefaults standardUserDefaults] setObject:@"1" forKey:HARDWARE_UD_KEY];
    }
}

+ (NSString *)filterHTML:(NSString *)html {
    NSScanner * scanner = [NSScanner scannerWithString:html];
    NSString * text = nil;
    while([scanner isAtEnd]==NO)
    {
        //找到标签的起始位置
        [scanner scanUpToString:@"<" intoString:nil];
        //找到标签的结束位置
        [scanner scanUpToString:@">" intoString:&text];
        //替换字符
        html = [html stringByReplacingOccurrencesOfString:[NSString stringWithFormat:@"%@>",text] withString:@""];
    }
    return html;
}

@end






