//
//  AVSettingAndConfigController.m
//  AliPlayerDemo
//
//  Created by 汪宁 on 2019/1/28.
//  Copyright © 2019年 com.alibaba. All rights reserved.
//

#import "AVSettingAndConfigController.h"
#import "CicadaConfig+refresh.h"
//track设置的枚举
typedef enum : NSUInteger {
    VideoTrack,
    AudioTrack,
    SubtitleTrack,
} TrackSetting;

@interface AVSettingAndConfigController()<NSComboBoxDelegate,NSComboBoxDataSource>
@property (weak) IBOutlet NSSlider *voiceSlider;
@property (weak) IBOutlet NSComboBox *scaleModelBox;
@property (weak) IBOutlet NSComboBox *speedModelBox;

@property (weak) IBOutlet NSComboBox *mirrorModelBox;
@property (weak) IBOutlet NSComboBox *rotationModelBox;

@property (weak) IBOutlet NSComboBox *videoSelectBox;
@property (weak) IBOutlet NSComboBox *audioSelectBox;
@property (weak) IBOutlet NSComboBox *subtitleSelectBox;
@property (strong) IBOutlet NSTextField *mediaInfoLabel;

@property (nonatomic, copy) NSString * mtsRegion;
@property (nonatomic, copy) NSString * videoId;
@property (nonatomic, copy) NSString * stsAccessKeyId;
@property (nonatomic, copy) NSString * stsAccessSecret;
@property (nonatomic, copy) NSString * stsSecurityToken;

//
@property (nonatomic, copy) NSView * configView;
@property (strong) IBOutlet NSTextField *startBufferTextField;

@property (strong) IBOutlet NSTextField *referTextField;
@property (strong) IBOutlet NSTextField *probeSizeTextField;
@property (strong) IBOutlet NSTextField *networkDelayTextField;
@property (strong) IBOutlet NSTextField *maxDelayTextField;

@property (strong) IBOutlet NSTextField *httpProxyTextField;
@property (strong) IBOutlet NSTextField *maxBufferTextField;
@property (strong) IBOutlet NSTextField *stopAndReviveTextField;

@property (weak) IBOutlet NSTextField *userAgentTextField;

@property (weak) IBOutlet NSTextField *retryCountTextField;

@property (strong) IBOutlet NSButton *showLastFrame;
@property (strong) IBOutlet NSButton *muteButton;

@end

@implementation AVSettingAndConfigController

- (NSView *)configView {
    if (!_configView) {
        NSNib *xib = [[NSNib alloc] initWithNibNamed:@"ConfigSetView" bundle:nil];
        NSArray *viewsArray = [[NSArray alloc] init];
        [xib instantiateWithOwner:self topLevelObjects:&viewsArray];
        for (int i = 0; i < viewsArray.count; i++) {
            if ([viewsArray[i] isKindOfClass:[NSView class]]) {
                _configView = viewsArray[i];
                break;
            }
        }
        _configView.frame = CGRectMake(400, 40, 350, 450);
        _configView.wantsLayer = YES;
        if (@available(macOS 10.13, *)) {
            _configView.layer.backgroundColor = [NSColor colorNamed:@"setColor"].CGColor;
        } else {
            _configView.layer.backgroundColor = [NSColor whiteColor].CGColor;
        }
        _configView.layer.borderColor = [NSColor whiteColor].CGColor;
        _configView.layer.borderWidth = 1;
        _configView.hidden = YES;
        [self.view.window.contentView addSubview:_configView];
    }
    return _configView;
}

- (IBAction)preparePlayer:(id)sender {

    [self.player prepare];
    [[NSNotificationCenter defaultCenter]postNotificationName:@"PlayerPrepare" object:nil];

}
- (IBAction)playVideo:(id)sender {
    [self.player start];
}

- (IBAction)pause:(id)sender {
    [self.player pause];
}

- (IBAction)stop:(id)sender {
    [self.player stop];
    self.topController.bufferPosition = 0;
    self.topController.currentPosition = 0;
    self.topController.allPosition = 0;
}

- (IBAction)screenShot:(id)sender {
    [self.player snapShot];
}

- (IBAction)mute:(id)sender {
    NSButton *button = sender;
    self.player.muted = button.state;
}

- (IBAction)autoPlay:(id)sender {
    NSButton *button = sender;
    self.player.autoPlay = button.state;
}

- (IBAction)circle:(id)sender {
    NSButton *button = sender;
    self.player.loop = button.state;
}

- (IBAction)voiceChange:(id)sender {
    NSSlider *slider = sender;
    self.player.volume = slider.floatValue / 100;
    if (self.player.volume == 0) {
        self.muteButton.state = YES;
        self.player.muted = YES;
    } else {
        self.muteButton.state = NO;
        self.player.muted = NO;
    }
}

- (void)setVolume:(CGFloat)value {
    self.player.volume = value;
}

- (IBAction)mediaInfo:(id)sender {
    CicadaMediaInfo *info = [self.player getMediaInfo];
    NSString *infoString = [HudTool infoStringDescribe:info];
    self.mediaInfoLabel.stringValue = infoString;
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(2 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        self.mediaInfoLabel.stringValue = @"";
    });
}

- (IBAction)setConfig:(id)sender {
    self.configView.hidden = NO;
}

- (void)addObject:(NSMutableArray *)array str:(NSString *)str {
    if (str && str.length >0) {
        [array addObject:str];
    }else{
        [array addObject:@""];
    }
}

- (NSArray *)getConfigArray {
    NSMutableArray *configArray = [NSMutableArray arrayWithCapacity:8];
    NSString * startBuffer = _startBufferTextField.stringValue;
    NSString * refer = _referTextField.stringValue;
    NSString * probeSize = _probeSizeTextField.stringValue;
    NSString * networkDelay = _networkDelayTextField.stringValue;
    NSString * maxDelay =_maxDelayTextField.stringValue;
    NSString * httpProxy =_httpProxyTextField.stringValue;
    NSString * maxBuffer =_maxBufferTextField.stringValue;
    NSString * stopAndRevive =_stopAndReviveTextField.stringValue;
    NSString * userAgent =_userAgentTextField.stringValue;
    NSString * retryCount =_retryCountTextField.stringValue;
    [self addObject:configArray str:startBuffer];
    [self addObject:configArray str:stopAndRevive];
    [self addObject:configArray str:maxBuffer];
    [self addObject:configArray str:maxDelay];
    [self addObject:configArray str:networkDelay];
    [self addObject:configArray str:probeSize];
    [self addObject:configArray str:refer];
    [self addObject:configArray str:httpProxy];
    [self addObject:configArray str:userAgent];
    [self addObject:configArray str:retryCount];
    if (_showLastFrame.state == NSControlStateValueOn) {
        [configArray addObject:@"1"];
    }else {
        [configArray addObject:@"0"];
    }
    return configArray.copy;
}

- (IBAction)config:(id)sender {
    self.configView.hidden = YES;
    
    NSArray *configArray = [self getConfigArray];
    CicadaConfig *config = [self.player getConfig];
    [config refreshConfigWithArray:configArray];
    [self.player setConfig:config];
    [HudTool hudWithText:@"使用成功"];
}

- (IBAction)closeConfigView:(id)sender {
    self.configView.hidden = YES;
}

#pragma Mark combo box 代理
- (NSArray *)getComboBoxArray:(NSComboBox *)comboBox {
    if (comboBox == self.videoSelectBox) {
        return self.videoTracksArray;
    }
    else if (comboBox == self.audioSelectBox) {
        return self.audioTracksArray;
    }
    else if (comboBox == self.subtitleSelectBox) {
        return self.subtitleTracksArray;
    }

    return nil;
}

-(void) selectTrackByNSComboBox:(NSComboBox*)box {
    NSArray *array = [self getComboBoxArray:box];
    NSInteger selectedIndex = [box indexOfSelectedItem];
    if (nil == array || selectedIndex >= array.count) {
        return;
    }

    CicadaTrackInfo *info = array[selectedIndex];
    [self.player selectTrack:info.trackIndex];
}

- (void)comboBoxSelectionDidChange:(NSNotification *)notification {
    if (notification.object == self.scaleModelBox) {
        switch ([self.scaleModelBox indexOfSelectedItem]) {
            case 0: { self.player.scalingMode = CICADA_SCALINGMODE_SCALEASPECTFIT; }
                break;
            case 1: { self.player.scalingMode = CICADA_SCALINGMODE_SCALEASPECTFILL; }
                break;
            case 2: { self.player.scalingMode = CICADA_SCALINGMODE_SCALETOFILL; }
                break;
            default:
                break;
        }
        return;
    }
    else if (notification.object == self.mirrorModelBox) {
        switch ([self.mirrorModelBox indexOfSelectedItem]) {
            case 0: { self.player.mirrorMode = CICADA_MIRRORMODE_NONE; }
                break;
            case 1: { self.player.mirrorMode = CICADA_MIRRORMODE_VERTICAL; }
                break;
            case 2: { self.player.mirrorMode = CICADA_MIRRORMODE_HORIZONTAL; }
                break;
            default:
                break;
        }
        return;
    }
    else if (notification.object == self.speedModelBox){
        NSInteger selectedIndex =  [self.speedModelBox indexOfSelectedItem];
        self.player.rate = 0.5 + 0.5 * selectedIndex;
        return;
    }
    else if(notification.object == self.rotationModelBox) {
        switch ([self.rotationModelBox indexOfSelectedItem]) {
            case 0: { self.player.rotateMode = CICADA_ROTATE_0; }
                break;
            case 1: { self.player.rotateMode = CICADA_ROTATE_90; }
                break;
            case 2: { self.player.rotateMode = CICADA_ROTATE_180; }
                break;
            case 3: { self.player.rotateMode = CICADA_ROTATE_270; }
                break;
            default:
                break;
        }
        return;
    }
    else if ((notification.object == self.videoSelectBox)
             || (notification.object == self.audioSelectBox)
             || (notification.object == self.subtitleSelectBox))
    {
        [self selectTrackByNSComboBox:notification.object];
    }
}

#pragma mark combobox dataDelegate

- (NSInteger)numberOfItemsInComboBox:(NSComboBox *)comboBox {
    NSArray *array = [self getComboBoxArray:comboBox];
    return array.count;
}

- (nullable id)comboBox:(NSComboBox *)comboBox objectValueForItemAtIndex:(NSInteger)index {
    NSArray *array = [self getComboBoxArray:comboBox];
    if (array.count > 0) {
        CicadaTrackInfo *info = array[index];
        if (nil == info.description || 0 == [info.description length]) {
            return [NSString stringWithFormat:@"%d", info.trackBitrate];
        }
        return info.description;
    }
    return nil;

}

- (void)setVideoTracksArray:(NSMutableArray *)array {
    _videoTracksArray = array;
    [self.videoSelectBox reloadData];
    if (array.count >0) {
        [_videoSelectBox selectItemAtIndex:0];
    }

}

- (void)setAudioTracksArray:(NSMutableArray *)array {
    _audioTracksArray = array;
    [self.audioSelectBox reloadData];
    if (array.count >0) {
         [_audioSelectBox selectItemAtIndex:0];
    }

}

- (void)setSubtitleTracksArray:(NSMutableArray *)array {
    _subtitleTracksArray = array;
    [self.subtitleSelectBox reloadData];
     [_subtitleSelectBox selectItemAtIndex:0];
}

- (void)setTotalDataArray:(NSArray *)totalDataArray{
    _totalDataArray = totalDataArray;
    if (_totalDataArray.count == 0){
        self.audioSelectBox.hidden = YES;
        self.videoSelectBox.hidden = YES;
        self.subtitleSelectBox.hidden = YES;
    }else{
        self.audioSelectBox.hidden = NO;
        self.videoSelectBox.hidden = NO;
        self.subtitleSelectBox.hidden = NO;
    }
}

- (void)setBoxDelegate {
    self.scaleModelBox.delegate = self;
    self.speedModelBox.delegate = self;
    [self.speedModelBox selectItemAtIndex:1];
    self.rotationModelBox.delegate = self;
    self.mirrorModelBox.delegate = self;

    self.audioSelectBox.delegate = self;
    self.videoSelectBox.delegate = self;
    self.subtitleSelectBox.delegate = self;

    self.audioSelectBox.usesDataSource = YES;
    self.videoSelectBox.usesDataSource = YES;
    self.subtitleSelectBox.usesDataSource = YES;

    self.audioSelectBox.dataSource = self;
    self.videoSelectBox.dataSource = self;
    self.subtitleSelectBox.dataSource = self;
    self.voiceSlider.floatValue = 100;
}

@end
