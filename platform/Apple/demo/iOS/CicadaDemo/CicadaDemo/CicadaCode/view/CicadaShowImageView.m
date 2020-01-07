//
//  CicadaShowImageView.m
//  CicadaPlayerDemo
//
//  Created by 郦立 on 2019/1/11.
//  Copyright © 2019年 com.alibaba. All rights reserved.
//

#import "CicadaShowImageView.h"
#import <Photos/Photos.h>

@implementation CicadaShowImageView

+ (void)showWithImage:(UIImage *)image inView:(UIView *)view {
    if (!image) {
        [CicadaTool hudWithText:NSLocalizedString(@"截图为空" , nil) view:view];
        return;
    }
    CicadaShowImageView *showImageView = [[CicadaShowImageView alloc]initWithFrame:view.bounds andImage:image];
    [view addSubview:showImageView];
    if (@available(iOS 9.0, *)) {
        [showImageView tryToSaveImage:image];
    }else {
        [showImageView saveMyFavoriteImageToCustomAlbum:image];
    }
}

- (instancetype)initWithFrame:(CGRect)frame andImage:(UIImage *)image {
    self = [super initWithFrame:frame];
    if (self) {

        self.backgroundColor = [UIColor colorWithWhite:0.3 alpha:1];
        
        UIImageView *imageView = [[UIImageView alloc]init];
        imageView.image = image;
        [imageView sizeToFit];
        imageView.center = self.center;
        [self addSubview:imageView];
        
        [self addTapGesture];
        
    }
    return self;
}

- (void)addTapGesture{
    UITapGestureRecognizer *tapGesture = [[UITapGestureRecognizer alloc]init];
    tapGesture.numberOfTapsRequired = 1;
    [tapGesture addTarget:self action:@selector(tap)];
    [self addGestureRecognizer:tapGesture];
}

- (void)tap {
    [self removeFromSuperview];
}

- (void)tryToSaveImage:(UIImage *)image {
    PHAuthorizationStatus status = [PHPhotoLibrary authorizationStatus];
    if (status == PHAuthorizationStatusRestricted) {
        [CicadaTool hudWithText:NSLocalizedString(@"因为系统原因, 保存到相册失败" , nil) view:self];
    } else if (status == PHAuthorizationStatusDenied) {
        [CicadaTool hudWithText:NSLocalizedString(@"因为系统原因, 保存到相册失败" , nil) view:self];
    } else if (status == PHAuthorizationStatusAuthorized) {
        [self saveImage:image];
    } else if (status == PHAuthorizationStatusNotDetermined) {
        [PHPhotoLibrary requestAuthorization:^(PHAuthorizationStatus status) {
            if (status == PHAuthorizationStatusAuthorized) {
                [self saveImage:image];
            }else {
                [CicadaTool hudWithText:NSLocalizedString(@"因为系统原因, 保存到相册失败" , nil) view:self];
            }
            dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(3 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
                [self removeFromSuperview];
            });
        }];
    }
    if (status != PHAuthorizationStatusNotDetermined) {
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(3 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            [self removeFromSuperview];
        });
    }
}

- (void)saveImage:(UIImage *)image {
    // PHAsset : 一个资源, 比如一张图片\一段视频
    // PHAssetCollection : 一个相簿
    // PHAsset的标识, 利用这个标识可以找到对应的PHAsset对象(图片对象)
    __block NSString *assetLocalIdentifier = nil;
    
    // 如果想对"相册"进行修改(增删改), 那么修改代码必须放在[PHPhotoLibrary sharedPhotoLibrary]的performChanges方法的block中
    [[PHPhotoLibrary sharedPhotoLibrary] performChanges:^{
        // 1.保存图片A到"相机胶卷"中
        // 创建图片的请求
        if (@available(iOS 9.0, *)) {
            assetLocalIdentifier = [PHAssetCreationRequest creationRequestForAssetFromImage:image].placeholderForCreatedAsset.localIdentifier;
        }
    } completionHandler:^(BOOL success, NSError * _Nullable error) {
        if (success == NO) {
            [CicadaTool hudWithText:NSLocalizedString(@"保存图片失败!" , nil) view:self];
            return;
        }
        
        // 2.获得相簿
        PHAssetCollection *createdAssetCollection = [self createdAssetCollection];
        if (createdAssetCollection == nil) {
            [CicadaTool hudWithText:NSLocalizedString(@"创建相簿失败!" , nil) view:self];
            return;
        }
        
        [[PHPhotoLibrary sharedPhotoLibrary] performChanges:^{
            // 3.添加"相机胶卷"中的图片A到"相簿"D中
            
            // 获得图片
            PHAsset *asset = [PHAsset fetchAssetsWithLocalIdentifiers:@[assetLocalIdentifier] options:nil].lastObject;
            
            // 添加图片到相簿中的请求
            PHAssetCollectionChangeRequest *request = [PHAssetCollectionChangeRequest changeRequestForAssetCollection:createdAssetCollection];
            
            // 添加图片到相簿
            [request addAssets:@[asset]];
        } completionHandler:^(BOOL success, NSError * _Nullable error) {
            if (success == NO) {
                [CicadaTool hudWithText:NSLocalizedString(@"保存图片失败!" , nil) view:self];
            } else {
                [CicadaTool hudWithText:NSLocalizedString(@"保存图片成功!" , nil) view:self];
            }
        }];
    }];
}

- (PHAssetCollection *)createdAssetCollection {
    // 从已存在相簿中查找这个应用对应的相簿
    PHFetchResult<PHAssetCollection *> *assetCollections = [PHAssetCollection fetchAssetCollectionsWithType:PHAssetCollectionTypeAlbum subtype:PHAssetCollectionSubtypeAlbumRegular options:nil];
    for (PHAssetCollection *assetCollection in assetCollections) {
        if ([assetCollection.localizedTitle isEqualToString:@"相机胶卷"]) {
            return assetCollection;
        }
    }
    
    // 没有找到对应的相簿, 得创建新的相簿
    
    // 错误信息
    NSError *error = nil;
    
    // PHAssetCollection的标识, 利用这个标识可以找到对应的PHAssetCollection对象(相簿对象)
    __block NSString *assetCollectionLocalIdentifier = nil;
    [[PHPhotoLibrary sharedPhotoLibrary] performChangesAndWait:^{
        // 创建相簿的请求
        assetCollectionLocalIdentifier = [PHAssetCollectionChangeRequest creationRequestForAssetCollectionWithTitle:@"相机胶卷"].placeholderForCreatedAssetCollection.localIdentifier;
    } error:&error];
    
    // 如果有错误信息
    if (error) return nil;
    
    // 获得刚才创建的相簿
    return [PHAssetCollection fetchAssetCollectionsWithLocalIdentifiers:@[assetCollectionLocalIdentifier] options:nil].lastObject;
}

#pragma mark iOS8

- (void)saveMyFavoriteImageToCustomAlbum:(UIImage *)image {
    //判断有没有访问相册的权限
    PHAuthorizationStatus oldStatus = [PHPhotoLibrary authorizationStatus];
    if (oldStatus != PHAuthorizationStatusAuthorized)
    {
        //申请访问相册的权限
        [PHPhotoLibrary requestAuthorization:^(PHAuthorizationStatus status) {
            dispatch_async(dispatch_get_main_queue(), ^{
                if (status != PHAuthorizationStatusAuthorized)
                {
                    [CicadaTool hudWithText:NSLocalizedString(@"没有权限" , nil) view:self];
                }
                else
                {
                    [self savedPhotoToAlbumWithImage:image album:@"相机胶卷"];
                }
            });
        }];
    }
    else
    {
        NSLog(@"有权限");
        [self savedPhotoToAlbumWithImage:image album:@"相机胶卷"];
    }
}

#pragma mark - 适配iOS8以上存储图片
//把图片存入指定的相册
- (void)savedPhotoToAlbumWithImage:(UIImage *)image album:(NSString *)photoAlbum
{
    //判断图片是不是为空
    if (image == nil || [PHPhotoLibrary authorizationStatus] != PHAuthorizationStatusAuthorized)
    {
        return;
    }
    //存储图片
    PHFetchResult<PHAsset *> *createdAssets = [self loadPhotoAssetsWithImage:image];
    if (createdAssets == nil)
    {
        //保存图片失败！
        [CicadaTool hudWithText:NSLocalizedString(@"保存图片失败" , nil) view:self];
        return;
    }
    //创建相册
    PHAssetCollection *createdCollection = [self loadAlbumWithName:photoAlbum];
    if (createdCollection == nil)
    {
        [CicadaTool hudWithText:NSLocalizedString(@"创建或者获取相册失败" , nil) view:self];
        //创建或者获取相册失败！
        return;
    }
    
    NSError *error = nil;
    //执行存储图片和创建相册
    [[PHPhotoLibrary sharedPhotoLibrary] performChangesAndWait:^{
        PHAssetCollectionChangeRequest *request = [PHAssetCollectionChangeRequest changeRequestForAssetCollection:createdCollection];
        [request insertAssets:createdAssets atIndexes:[NSIndexSet indexSetWithIndex:0]];
    } error:&error];
}

//创建一个相册
- (PHAssetCollection *)loadAlbumWithName:(NSString *)albumName
{
    NSString *albumNameTitle = (albumName.length != 0)? albumName : @"";
    //抓取所有的自定义相册（保证相册只被创建一个）
    PHFetchResult<PHAssetCollection *> *collections = [PHAssetCollection fetchAssetCollectionsWithType:PHAssetCollectionTypeAlbum subtype:PHAssetCollectionSubtypeAlbumRegular options:nil];
    PHAssetCollection *createdCollection = nil;
    for (PHAssetCollection *collection in collections)
    {
        if ([collection.localizedTitle isEqualToString:albumNameTitle])
        {
            createdCollection = collection;
            break;
        }
    }
    //当前APP对应的自定义相册没有被创建过
    NSError *error = nil;
    if (createdCollection == nil)
    {
        __block NSString *createdCollectionID;
        [[PHPhotoLibrary sharedPhotoLibrary] performChangesAndWait:^{
            //创建一个相册,拿到相册的唯一标识符
            createdCollectionID = [PHAssetCollectionChangeRequest creationRequestForAssetCollectionWithTitle:albumNameTitle].placeholderForCreatedAssetCollection.localIdentifier;
        } error:&error];
        
        //根据相册的唯一标识符拿到相册
        createdCollection = [PHAssetCollection fetchAssetCollectionsWithLocalIdentifiers:@[createdCollectionID] options:nil].firstObject;
    }
    return createdCollection;
}

//保存图片
- (PHFetchResult<PHAsset *> *)loadPhotoAssetsWithImage:(UIImage *)saveImage
{
    NSError *error = nil;
    __block NSString *assetID = nil;
    [[PHPhotoLibrary sharedPhotoLibrary] performChangesAndWait:^{//同步
        assetID = [PHAssetChangeRequest creationRequestForAssetFromImage:saveImage].placeholderForCreatedAsset.localIdentifier;
    } error:&error];
    if (error)
    {
        [CicadaTool hudWithText:NSLocalizedString(@"保存图片失败" , nil) view:self];
        return nil;
    }
    [CicadaTool hudWithText:NSLocalizedString(@"保存图片成功" , nil) view:self];
    return [PHAsset fetchAssetsWithLocalIdentifiers:@[assetID] options:nil];
}

@end








