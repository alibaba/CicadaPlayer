//
//  AVPShowImageView.m
//  AliPlayerDemo
//
//  Created by 郦立 on 2019/1/11.
//  Copyright © 2019年 com.alibaba. All rights reserved.
//

#import "AVPShowImageView.h"
#import <Photos/Photos.h>

@interface AVPShowImageView ()

@property(nonatomic, strong) NSImageView *imageView;

@end

@implementation AVPShowImageView

+ (void)showWithImage:(NSImage *)image inView:(NSView *)view {
    if (!image) {
        [HudTool hudWithText:@"截图为空"];
        return;
    }
 
    AVPShowImageView *showImageView = [[AVPShowImageView alloc]initWithFrame:CGRectMake(view.bounds.size.width/2, 150, view.bounds.size.width/2, view.bounds.size.height - 150) andImage:image];
    [view addSubview:showImageView];
    
    [showImageView tryToSaveImage:image];
    

}

- (instancetype)initWithFrame:(CGRect)frame andImage:(NSImage *)image {
    self = [super initWithFrame:frame];
    if (self) {
        self.wantsLayer = YES;
        self.layer.backgroundColor = [NSColor colorWithRed:0 green:0 blue:0 alpha:0.6].CGColor;//[UIColor colorWithWhite:0.3 alpha:1];
        
        _imageView = [[NSImageView alloc]init];
        _imageView.image = image;
        [_imageView sizeToFit];
        _imageView.frame = CGRectMake(0, 0, frame.size.width, frame.size.height);
        [self addSubview:_imageView];
        
        
    }
    return self;
}


- (void)mouseDown:(NSEvent *)event {
   
    [self removeFromSuperview];
}

- (void)tryToSaveImage:(NSImage *)image {
    
    NSSavePanel *panel = [NSSavePanel savePanel];
    panel.title = @"保存图片";
    [panel setMessage:@"选择图片保存地址"];//提示文字
    
    [panel setDirectoryURL:[NSURL fileURLWithPath:[NSHomeDirectory() stringByAppendingPathComponent:@"Pictures"]]];//设置默认打开路径
    
    [panel setNameFieldStringValue:@"截图"];
    [panel setAllowsOtherFileTypes:YES];
    [panel setAllowedFileTypes:@[@"jpg",@"png"]];
    [panel setExtensionHidden:NO];
    [panel setCanCreateDirectories:YES];
    
    [panel beginSheetModalForWindow:self.window completionHandler:^(NSInteger result){
        if (result == NSModalResponseOK)
        {
            NSString *path = [[panel URL] path];
            NSData *tiffData = [self.imageView.image TIFFRepresentation];
            [tiffData writeToFile:path atomically:YES];
        }
    }];
    
}


@end








