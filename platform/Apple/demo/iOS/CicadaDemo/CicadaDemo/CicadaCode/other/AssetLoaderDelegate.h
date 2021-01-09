//
//  AssetLoaderDelegate.h
//  iOS FPS Client AxinomDRM
//
//  Created by Axinom.
//  Copyright (c) Axinom. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface AssetLoaderDelegate : NSObject <AVAssetResourceLoaderDelegate>

@property (nonatomic, strong) NSString *certificateUrl;
@property (nonatomic, strong) NSString *licenseUrl;

- (instancetype)initWithCertificateUrl:(NSString *)certificateUrl licenseUrl:(NSString *)licenseUrl;

@end
