//
//  MutlPlayViewController.m
//  CicadaDemo
//
//  Created by ToT on 2020/4/23.
//  Copyright © 2020 com.alibaba. All rights reserved.
//

#import "MutlPlayViewController.h"
#import "AppDelegate.h"
#import <CicadaPlayerSDK/CicadaSyncPlayerServer.h>
#import <CicadaPlayerSDK/CicadaSyncPlayerClient.h>

@interface MutlPlayViewController ()<CicadaDelegate>

@property (nonatomic,strong)NSMutableArray *playViewArray;
@property (nonatomic,strong)NSMutableArray *playersArray;
@property (nonatomic,assign)BOOL isFullScreen;
@property (nonatomic,assign)NSView *fullScreenView;
@property (nonatomic,assign)NSInteger viewWidth;
@property (nonatomic,assign)NSInteger viewHeight;
@property (nonatomic,strong)NSView *horizontalLine;
@property (nonatomic,strong)NSView *verticalLine;

@end

@implementation MutlPlayViewController

- (NSInteger)viewWidth {
    AppDelegate *appdelegate = (AppDelegate *) [NSApplication sharedApplication].delegate;
    NSWindow *window = appdelegate.mainWindow.window;
    return window.frame.size.width;
}

- (NSInteger)viewHeight {
    AppDelegate *appdelegate = (AppDelegate *) [NSApplication sharedApplication].delegate;
    NSWindow *window = appdelegate.mainWindow.window;
    return window.frame.size.height-22;
}

- (void)loadView {
    NSView *view = [[NSView alloc]initWithFrame:NSMakeRect(0, 0, self.viewWidth, self.viewHeight)];
    self.view = view;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do view setup here.
}

- (void)viewDidAppear {
    [super viewDidAppear];
    
    self.horizontalLine = [[NSView alloc]init];
    self.horizontalLine.wantsLayer = true;
    self.horizontalLine.layer.backgroundColor = [NSColor redColor].CGColor;
    [self.horizontalLine setNeedsDisplay:YES];
    [self.view addSubview:self.horizontalLine];
    
    self.verticalLine = [[NSView alloc]init];
    self.verticalLine.wantsLayer = true;
    self.verticalLine.layer.backgroundColor = [NSColor redColor].CGColor;
    [self.verticalLine setNeedsDisplay:YES];
    [self.view addSubview:self.verticalLine];
    
    self.playersArray = [NSMutableArray array];
    self.playViewArray = [NSMutableArray array];
    for (int i = 0; i<4; i++) {
        NSView *view = [[NSView alloc]init];
        view.wantsLayer = true;
        [view setNeedsDisplay:YES];
        
        [self.view addSubview:view];
        [self.playViewArray addObject:view];
    }
    
    NSPressGestureRecognizer *tap = [[NSPressGestureRecognizer alloc]initWithTarget:self action:@selector(viewTouches:)];
    tap.minimumPressDuration = 0.01;
    [self.view addGestureRecognizer:tap];
    
    [self displayPrivateViews];
    
    if (self.urlSourceArray.count == 4) {
        for (int i = 0; i<4; i++) {
            CicadaPlayer *player = nil;
            if (0 == i) {
                player = [[CicadaSyncPlayerServer alloc] init];
            } else {
                player = [[CicadaSyncPlayerClient alloc]init];
            }

            [player setUrlSource:self.urlSourceArray[i]];
            player.autoPlay = YES;
            player.loop = YES;
            player.delegate = self;
            player.playerView = self.playViewArray[i];
            [player prepare];
            [self.playersArray addObject:player];
        }
    }
}

- (void)viewTouches:(NSPressGestureRecognizer *)recognizer {
    if (recognizer.state == NSGestureRecognizerStateBegan) {
        self.isFullScreen = !self.isFullScreen;
        if (self.isFullScreen) {
            NSPoint point = [recognizer locationInView:self.view];
            NSInteger viewIndex = 0;
            if (point.y > self.viewHeight/2) {
                viewIndex = viewIndex + 2;
            }
            if (point.x > self.viewWidth/2) {
                viewIndex = viewIndex + 1;
            }
            NSView *view = self.playViewArray[viewIndex];
            [view removeFromSuperview];
            view.frame = self.view.bounds;
            [self.view addSubview:view];
            [view setNeedsDisplay:YES];
            self.fullScreenView = view;
        }
        [self displayPrivateViews];
    }
}

- (void)viewDidDisappear {
    [super viewDidDisappear];
    
    for (CicadaPlayer *player in self.playersArray) {
        [player stop];
        [player destroy];
    }
    [self.playersArray removeAllObjects];
}

- (void)viewDidLayout {
    [super viewDidLayout];
    
    [self displayPrivateViews];
}

- (void)displayPrivateViews {
    self.view.frame = NSMakeRect(0, 0, self.viewWidth, self.viewHeight);
    [self.view setNeedsDisplay:YES];
    if (self.isFullScreen) {
        self.fullScreenView.frame = self.view.bounds;
        [self.fullScreenView setNeedsDisplay:YES];
    }else {
        for (int i = 0; i<4; i++) {
            NSView *view = self.playViewArray[i];
            view.frame = NSMakeRect((i%2)*(self.viewWidth/2+1), (i/2)*(self.viewHeight/2+1), (self.viewWidth-2)/2, (self.viewHeight-2)/2);
            [view setNeedsDisplay:YES];
        }
    }
    self.horizontalLine.frame = NSMakeRect(0, self.viewHeight/2-1, self.viewWidth, 2);
    [self.horizontalLine setNeedsDisplay:YES];
    self.verticalLine.frame = NSMakeRect(self.viewWidth/2-1, 0, 2, self.viewHeight);
    [self.verticalLine setNeedsDisplay:YES];
}

#pragma mark CicadaDelegate

- (void)onTrackReady:(CicadaPlayer *)player info:(NSArray<CicadaTrackInfo *> *)info {
    //选择不同清晰度播放
//    NSMutableArray * videoTracksArray = [NSMutableArray array];
//    for (int i=0; i<info.count; i++) {
//        CicadaTrackInfo* track = [info objectAtIndex:i];
//        switch (track.trackType) {
//            case CICADA_TRACK_VIDEO: {
//                [videoTracksArray addObject:track];
//            }
//                break;
//            default:
//                break;
//        }
//    }
//    NSInteger playerIndex = [self.playersArray indexOfObject:player];
//    NSInteger trackInfoIndex = playerIndex % videoTracksArray.count;
//    CicadaTrackInfo *track = videoTracksArray[trackInfoIndex];
//    [player selectTrack:track.trackIndex];
}

@end
