//
//  AssetLoaderDelegate.m
//  iOS FPS Client AxinomDRM
//
//  Created by Axinom.
//  Copyright (c) Axinom. All rights reserved.
//

#import <AVFoundation/AVFoundation.h>
#import "AssetLoaderDelegate.h"

typedef void(^AppCertificateRequestCompletion)(NSData *certificate);
typedef void(^ContentKeyAndLeaseExpiryRequestCompletion)(NSData *response, NSError *error);

@interface AssetLoaderDelegate ()

@property (nonatomic, strong) NSData *certificateData;

@end

@implementation AssetLoaderDelegate

- (instancetype)initWithCertificateUrl:(NSString *)certificateUrl licenseUrl:(NSString *)licenseUrl {
    self = [super init];
    if (self) {
        self.certificateUrl = certificateUrl;
        self.licenseUrl = licenseUrl;
        [self requestApplicationCertificateWithCompletion:^(NSData *certificate) {
            self.certificateData = certificate;
        }];
    }
    return self;
}

- (BOOL)resourceLoader:(AVAssetResourceLoader *)resourceLoader shouldWaitForRenewalOfRequestedResource:(AVAssetResourceRenewalRequest *)renewalRequest {
    return [self resourceLoader:resourceLoader shouldWaitForLoadingOfRequestedResource:renewalRequest];
}

- (BOOL)resourceLoader:(AVAssetResourceLoader *)resourceLoader shouldWaitForLoadingOfRequestedResource:(AVAssetResourceLoadingRequest *)loadingRequest {
    NSURL *url = loadingRequest.request.URL;
    if (![[url scheme] isEqual:@"skd"]) {
        return NO;
    }
    AVAssetResourceLoadingDataRequest *dataRequest = loadingRequest.dataRequest;
    NSString *assetStr = [url.absoluteString stringByReplacingOccurrencesOfString:@"skd://" withString:@""];
    NSData *assetId = [NSData dataWithBytes: [assetStr cStringUsingEncoding:NSUTF8StringEncoding] length:[assetStr lengthOfBytesUsingEncoding:NSUTF8StringEncoding]];
    NSError *error = nil;
    NSData *requestBytes = [loadingRequest streamingContentKeyRequestDataForApp:self.certificateData contentIdentifier:assetId options:nil error:&error];
    [self requestContentKeyAndLeaseExpiryfromKeyServerModuleWithRequestBytes:requestBytes
                                                                     assetId:assetStr
                                                         completion:^(NSData *response, NSError *error) {
                                                             if (response) {
                                                                 [dataRequest respondWithData:response];
                                                                 if (@available(iOS 9.0, *)) {
                                                                     loadingRequest.contentInformationRequest.contentType = AVStreamingKeyDeliveryContentKeyType;
                                                                 } else {
                                                                     
                                                                 }
                                                                 [loadingRequest finishLoading];
                                                             }
                                                             else {
                                                                 [loadingRequest finishLoadingWithError:error];
                                                             }
                                                         }];
    return YES;
}

- (void)requestApplicationCertificateWithCompletion:(AppCertificateRequestCompletion)completion {
    NSURLSessionConfiguration *configuration = [NSURLSessionConfiguration defaultSessionConfiguration];
    NSURLSession *session = [NSURLSession sessionWithConfiguration:configuration];
    NSURL *url = [NSURL URLWithString:self.certificateUrl];
    NSURLSessionDataTask *requestTask = [session dataTaskWithURL:url completionHandler:^(NSData * _Nullable data, NSURLResponse * _Nullable response, NSError * _Nullable error) {
        completion(data);
    }];
    [requestTask resume];
}

- (void)requestContentKeyAndLeaseExpiryfromKeyServerModuleWithRequestBytes:(NSData *)requestBytes assetId:(NSString *)assetId completion:(ContentKeyAndLeaseExpiryRequestCompletion)completion {
    NSURLSessionConfiguration *configuration = [NSURLSessionConfiguration defaultSessionConfiguration];
    NSURLSession *session = [NSURLSession sessionWithConfiguration:configuration];
    NSURL *url = [NSURL URLWithString:self.licenseUrl];
    NSMutableURLRequest *ksmRequest = [NSMutableURLRequest requestWithURL:url];
    [ksmRequest setHTTPMethod:@"POST"];
    
    NSString *spc = [requestBytes base64EncodedStringWithOptions:NSDataBase64EncodingEndLineWithLineFeed];
    NSString *postStr = [NSString stringWithFormat:@"spc=%@&assetId=%@", spc, assetId];
    NSData *postData = [postStr dataUsingEncoding:NSASCIIStringEncoding allowLossyConversion:YES];
    [ksmRequest setHTTPBody:postData];

    NSURLSessionDataTask *requestTask = [session dataTaskWithRequest:ksmRequest completionHandler:^(NSData * _Nullable data, NSURLResponse * _Nullable response, NSError * _Nullable error) {
        NSString *responseStr = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
        responseStr = [responseStr stringByReplacingOccurrencesOfString:@"<ckc>" withString:@""];
        responseStr = [responseStr stringByReplacingOccurrencesOfString:@"</ckc>" withString:@""];
        NSData* decodeData = [[NSData alloc] initWithBase64EncodedString:responseStr options:0];
        completion(decodeData, error);
    }];
    [requestTask resume];
}

@end
