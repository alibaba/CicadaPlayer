//
//  CicadaErrorModel+string.m
//  CicadaPlayerDemo
//
//  Created by 郦立 on 2019/1/19.
//  Copyright © 2019年 com.alibaba. All rights reserved.
//

#import "CicadaErrorModel+string.h"

@implementation CicadaErrorModel (string)

- (NSString *)errorString {
    int tmpid = self.code;
    NSString *nLetterValue;
    NSString *backstr = @"";
    long long int ttmpig;
    for (int i = 0; i<9; i++) {
        ttmpig=tmpid%16;
        tmpid=tmpid/16;
        switch (ttmpig) {
            case 10:
                nLetterValue = @"A";break;
            case 11:
                nLetterValue = @"B";break;
            case 12:
                nLetterValue = @"C";break;
            case 13:
                nLetterValue = @"D";break;
            case 14:
                nLetterValue = @"E";break;
            case 15:
                nLetterValue = @"F";break;
            default:nLetterValue=[[NSString alloc]initWithFormat:@"%lli",ttmpig];
        }
        backstr = [nLetterValue stringByAppendingString:backstr];
        if (tmpid == 0) { break; }
    }
    return [NSString stringWithFormat:@"errorCode:%lu;%@:%@;errorMessage:%@",(unsigned long)self.code,NSLocalizedString(@"十六进制" , nil),backstr,self.message];
}



@end
