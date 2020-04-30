//
//  RightView.h
//  AliPlayerDemo
//
//  Created by 汪宁 on 2019/2/15.
//  Copyright © 2019年 com.alibaba. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "RootViewController.h"
NS_ASSUME_NONNULL_BEGIN

typedef enum : NSUInteger {
    inputURL,
    collectionView,
    urlLinks,
    localURL,
    MultPlayer
} ViewType;

@interface RightView : NSView


@property (nonatomic, strong) RootViewController * vc;

- (void)loadViewWithViewType:(ViewType)viewType;
- (void)reloadCollectionViewWithArray:(NSArray *)array;

@end

NS_ASSUME_NONNULL_END
