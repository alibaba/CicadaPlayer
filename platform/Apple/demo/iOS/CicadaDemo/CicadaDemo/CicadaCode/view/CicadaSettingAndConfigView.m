//
//  CicadaSettingAndConfigView.m
//  CicadaPlayerDemo
//
//  Created by 郦立 on 2019/1/7.
//  Copyright © 2019年 com.alibaba. All rights reserved.
//

#import "CicadaSettingAndConfigView.h"
#import "UIView+AVPFrame.h"
#import "CicadaSlider.h"

static NSString * const tableViewCellIdentifier = @"UITableViewCell";

@interface CicadaSettingAndConfigView ()<UITabBarDelegate,UITableViewDelegate,UITableViewDataSource>

@property (nonatomic,strong)UITabBar *tabbar;
@property (nonatomic,assign)NSInteger currentIndex;
@property (nonatomic,strong)UIScrollView *containView0;
@property (nonatomic,strong)UIScrollView *containView1;
@property (nonatomic,strong)UIScrollView *containView2;
@property (nonatomic,strong)UITableView *tableView;
@property (nonatomic,strong)NSMutableArray *textFieldArray;
@property (nonatomic,strong)NSMutableArray *cacheTextFieldArray;
@property (nonatomic,strong)NSMutableArray *tableViewDataArray;
@property (nonatomic,strong)NSMutableArray *tableViewSelectedArray;
@property (nonatomic,strong)UISwitch * openCacheSwitch;
@property (nonatomic,strong)UISwitch * showWhenStopSwitch;
@property (nonatomic,strong)CicadaSlider *voiceSlider;
@property (nonatomic,assign)BOOL isAuto;
@property (nonatomic,strong)NSString *videoStr;
@property (nonatomic,strong)UISwitch *muteSwitch;
@property (nonatomic,strong)UISwitch *hardwareDecoderSwitch;
@property (nonatomic,strong)UISwitch *accurateSeekSwitch;
@property (nonatomic,strong)UISwitch *playBackgournd;
@end

@implementation CicadaSettingAndConfigView

- (instancetype)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        
        self.videoStr = @"";
        self.subtitleIndexDictionary = [NSMutableDictionary dictionary];
        self.backgroundColor = [UIColor colorWithWhite:0.88 alpha:1];
        [self resetTableViewData];
        
        CGFloat selfWidth = frame.size.width;
        CGFloat selfHeight = frame.size.height;
        CGFloat tabbarHeight = 49;
        
        self.tabbar = [[UITabBar alloc]initWithFrame:CGRectMake(0, selfHeight-tabbarHeight, selfWidth, tabbarHeight)];
        UITabBarItem *item0 = [[UITabBarItem alloc]initWithTitle:@"options" image:nil tag:10];
        item0.titlePositionAdjustment = UIOffsetMake(0, -18);
        UITabBarItem *item1 = [[UITabBarItem alloc]initWithTitle:@"playConfig" image:nil tag:11];
        item1.titlePositionAdjustment = UIOffsetMake(0, -18);
        UITabBarItem *item2 = [[UITabBarItem alloc]initWithTitle:@"cacheConfig" image:nil tag:12];
        item2.titlePositionAdjustment = UIOffsetMake(0, -18);
        UITabBarItem *item3 = [[UITabBarItem alloc]initWithTitle:@"track" image:nil tag:13];
        item3.titlePositionAdjustment = UIOffsetMake(0, -18);
        self.tabbar.items = @[item0,item1,item2,item3];
        self.tabbar.selectedItem = self.tabbar.items.firstObject;
        self.tabbar.delegate = self;
        [self addSubview:self.tabbar];
        self.currentIndex = 10;
        
        self.containView0 = [[UIScrollView alloc]initWithFrame:CGRectMake(0, 0, selfWidth, selfHeight-tabbarHeight)];
        self.containView0.showsVerticalScrollIndicator = NO;
        self.containView0.showsHorizontalScrollIndicator = NO;
        self.containView0.bounces = NO;
        self.containView1 = [[UIScrollView alloc]initWithFrame:CGRectMake(0, 0, selfWidth, selfHeight-tabbarHeight)];
        self.containView1.contentSize = CGSizeMake(selfWidth, selfHeight*2);
        self.containView1.showsVerticalScrollIndicator = NO;
        self.containView1.showsHorizontalScrollIndicator = NO;
        self.containView1.bounces = NO;
        self.containView2 = [[UIScrollView alloc]initWithFrame:CGRectMake(0, 0, selfWidth, selfHeight-tabbarHeight)];
        self.containView2.showsVerticalScrollIndicator = NO;
        self.containView2.showsHorizontalScrollIndicator = NO;
        self.containView2.bounces = NO;
        self.tableView = [[UITableView alloc]initWithFrame:CGRectMake(0, 0, selfWidth, selfHeight-tabbarHeight)];
        self.tableView.delegate = self;
        self.tableView.dataSource = self;
        self.tableView.backgroundColor = self.backgroundColor;
        [self addSubview:self.tableView];
        
        [self addSubview:self.containView0];
        [self addSubview:self.containView1];
        [self addSubview:self.containView2];
        [self addSubview:self.tableView];
        self.containView1.hidden = self.containView2.hidden = self.tableView.hidden = YES;
        
        //containView0
        NSArray * switchTitleArray = @[NSLocalizedString(@"自动播放" , nil),NSLocalizedString(@"静音" , nil),NSLocalizedString(@"循环" , nil),NSLocalizedString(@"硬解码" , nil),NSLocalizedString(@"精准seek" , nil),NSLocalizedString(@"后台播放" , nil)];
        for (int i = 0; i < switchTitleArray.count; i++) {
            UISwitch * addSwitch = [[UISwitch alloc]initWithFrame:CGRectMake(0, 0, 49, 31)];
            addSwitch.tag = i;
            addSwitch.center = CGPointMake(selfWidth/switchTitleArray.count/2 * (2*i+1), 30);
            [addSwitch addTarget:self action:@selector(switchDidChange:) forControlEvents:UIControlEventValueChanged];
            if (i == 1) {
                self.muteSwitch = addSwitch;
            }else if (i == 3) {
                addSwitch.on = YES;
                addSwitch.userInteractionEnabled = NO;
                self.hardwareDecoderSwitch = addSwitch;
            }else if (i == 4) {
                self.accurateSeekSwitch = addSwitch;
            } else if (i == 5) {
                self.playBackgournd = addSwitch;
            }

            [self.containView0 addSubview:addSwitch];
            
            UILabel *switchLabel = [self commonLabel];
            NSArray *languages = [NSLocale preferredLanguages];
            NSString *currentLanguage = [languages objectAtIndex:0];
            if ([currentLanguage containsString:@"en"]) {
                switchLabel.font = [UIFont systemFontOfSize:10];
            }
            switchLabel.frame = CGRectMake(selfWidth/switchTitleArray.count*i, 50, selfWidth/switchTitleArray.count, 20);
            switchLabel.text = switchTitleArray[i];
            switchLabel.textAlignment = NSTextAlignmentCenter;
            [self.containView0 addSubview:switchLabel];
        }
        
        NSArray *leadTitleArray = @[NSLocalizedString(@"音量" , nil),NSLocalizedString(@"缩放模式" , nil),NSLocalizedString(@"镜像模式" , nil),NSLocalizedString(@"旋转模式" , nil),NSLocalizedString(@"倍速播放" , nil)];
        NSArray *segAllTitleArray = @[@[NSLocalizedString(@"比例填充" , nil),NSLocalizedString(@"比例全屏" , nil),NSLocalizedString(@"拉伸全屏" , nil)],@[NSLocalizedString(@"无镜像" , nil),NSLocalizedString(@"水平镜像" , nil),NSLocalizedString(@"垂直镜像" , nil)],@[@"0°",@"90°",@"180°",@"270°"],@[NSLocalizedString(@"正常" , nil),NSLocalizedString(@"0.5倍速" , nil),NSLocalizedString(@"1.5倍速" , nil),NSLocalizedString(@"2倍速" , nil)]];
        CGFloat segMaxY = 0;
        for (int i = 0; i < leadTitleArray.count; i++) {
            UILabel *leadingLabel = [self commonLabel];
            leadingLabel.frame = CGRectMake(15, 80+40*i, 60, 40);
            leadingLabel.text = leadTitleArray[i];
            [self.containView0 addSubview:leadingLabel];
            if (i == leadTitleArray.count - 1) {
                segMaxY = leadingLabel.getMaxY;
            }
            if (i == 0) {
                self.voiceSlider = [[CicadaSlider alloc]initWithFrame:CGRectMake(85, 85, selfWidth-100, 30)];
                [self.voiceSlider addTarget:self action:@selector(voiceSliderDidChange:) forControlEvents:UIControlEventValueChanged];
                [self.containView0 addSubview:self.voiceSlider];
                UIView * line = [[UIView alloc]initWithFrame:CGRectMake(0, 0, 2, 30)];
                line.backgroundColor = [UIColor darkGrayColor];
                line.center = self.voiceSlider.center;
                [self.containView0 addSubview:line];
                [self.containView0 sendSubviewToBack:line];
            }else {
                NSArray *segTitleArray = segAllTitleArray[i-1];
                UISegmentedControl * segmentedControl = [[UISegmentedControl alloc]initWithItems:segTitleArray];
                segmentedControl.tag = i - 1;
                segmentedControl.frame = CGRectMake(85, 85 + i*40, selfWidth-100, 30);
                [segmentedControl addTarget:self action:@selector(segmentedControlDidChange:) forControlEvents:UIControlEventValueChanged];
                segmentedControl.selectedSegmentIndex = 0;
                [self.containView0 addSubview:segmentedControl];
            }
        }
        UIButton *mediaButton = [[UIButton alloc]initWithFrame:CGRectMake(selfWidth-120, segMaxY + 10, 120, 40)];
        [mediaButton setTitle:NSLocalizedString(@"媒体信息" , nil) forState:UIControlStateNormal];
        mediaButton.titleLabel.font = [UIFont systemFontOfSize:14];
        [mediaButton setTitleColor:self.tabbar.tintColor forState:UIControlStateNormal];
        [mediaButton setTitleColor:[UIColor lightGrayColor] forState:UIControlStateHighlighted];
        [mediaButton addTarget:self action:@selector(bottonButtonClick:) forControlEvents:UIControlEventTouchUpInside];
        [self.containView0 addSubview:mediaButton];
        self.containView0.contentSize = CGSizeMake(selfWidth, mediaButton.getMaxY+10);
        
        //containView1
        NSArray *textFieldTitleArray = @[NSLocalizedString(@"启播缓冲时长" , nil),NSLocalizedString(@"卡顿恢复" , nil),NSLocalizedString(@"最大缓冲时长" , nil),NSLocalizedString(@"直播最大延迟" , nil),NSLocalizedString(@"网络超时" , nil),NSLocalizedString(@"请求refer" , nil),NSLocalizedString(@"httpProxy代理" , nil),NSLocalizedString(@"重连次数" , nil)];
        self.textFieldArray = [NSMutableArray array];
        CGFloat txtMaxY = 0;
        for (int i = 0; i<textFieldTitleArray.count; i++) {
            UILabel *label = [self commonLabel];
            label.frame = CGRectMake(15, 40*i+10, 120, 40);
            label.text = textFieldTitleArray[i];
            [self.containView1 addSubview:label];
            UITextField *textField = [[UITextField alloc]initWithFrame:CGRectMake(140, 40*i+15, selfWidth-155, 30)];
            textField.textColor = [UIColor blackColor];
            textField.backgroundColor = [UIColor colorWithWhite:0.8 alpha:1];
            textField.layer.cornerRadius = 3;
            textField.leftView = [[UIView alloc]initWithFrame:CGRectMake(0, 0, 8, 0)];
            textField.leftViewMode = UITextFieldViewModeAlways;
            [self.containView1 addSubview:textField];
            [self.textFieldArray addObject:textField];
            if (i == textFieldTitleArray.count -1) {
                txtMaxY = label.getMaxY;
            }
        }
        
        UILabel *label = [self commonLabel];
        label.frame = CGRectMake(15, txtMaxY + 10, 120, 40);
        label.text = NSLocalizedString(@"停止隐藏最后帧" , nil);
        [self.containView1 addSubview:label];
        
        self.showWhenStopSwitch = [[UISwitch alloc]initWithFrame:CGRectMake(135, txtMaxY + 10, 49, 31)];
        [self.containView1 addSubview:self.showWhenStopSwitch];
        
        UIButton *refreshButton = [[UIButton alloc]initWithFrame:CGRectMake(selfWidth-120, txtMaxY + 10, 120, 40)];
        [refreshButton setTitle:NSLocalizedString(@"应用配置" , nil) forState:UIControlStateNormal];
        refreshButton.titleLabel.font = [UIFont systemFontOfSize:14];
        [refreshButton setTitleColor:self.tabbar.tintColor forState:UIControlStateNormal];
        [refreshButton setTitleColor:[UIColor lightGrayColor] forState:UIControlStateHighlighted];
        [refreshButton addTarget:self action:@selector(bottonButtonClick:) forControlEvents:UIControlEventTouchUpInside];
        refreshButton.tag = 1;
        [self.containView1 addSubview:refreshButton];
        self.containView1.contentSize = CGSizeMake(selfWidth, refreshButton.getMaxY+10);
        
        //containView2
        NSArray *cacheTextFieldTitleArray = @[NSLocalizedString(@"最大时长(单位s)" , nil),NSLocalizedString(@"最大size(单位MB)" , nil)];
        NSArray *dicKey = @[@"maxDuration",@"maxSizeMB"];
        self.cacheTextFieldArray = [NSMutableArray array];
        CGFloat cacheTxtMaxY = 0;
        for (int i = 0; i<cacheTextFieldTitleArray.count; i++) {
            UILabel *label = [self commonLabel];
            label.frame = CGRectMake(15, 40*i+10, 120, 40);
            label.text = cacheTextFieldTitleArray[i];
            [self.containView2 addSubview:label];
            UITextField *textField = [[UITextField alloc]initWithFrame:CGRectMake(140, 40*i+15, selfWidth-155, 30)];
            textField.textColor = [UIColor blackColor];
            textField.backgroundColor = [UIColor colorWithWhite:0.8 alpha:1];
            textField.layer.cornerRadius = 3;
            textField.leftView = [[UIView alloc]initWithFrame:CGRectMake(0, 0, 8, 0)];
            textField.leftViewMode = UITextFieldViewModeAlways;
            textField.text = [CicadaTool getConfigDictionary][dicKey[i]];
            [self.containView2 addSubview:textField];
            [self.cacheTextFieldArray addObject:textField];
            if (i == cacheTextFieldTitleArray.count -1) {
                cacheTxtMaxY = label.getMaxY;
            }
        }
        
        UILabel *cacheLabel = [self commonLabel];
        cacheLabel.frame = CGRectMake(15, cacheTxtMaxY + 10, 100, 40);
        cacheLabel.text = NSLocalizedString(@"开启缓存配置" , nil);
        [self.containView2 addSubview:cacheLabel];
        
        self.openCacheSwitch = [[UISwitch alloc]initWithFrame:CGRectMake(135, cacheTxtMaxY + 10, 49, 31)];
        [self.containView2 addSubview:self.openCacheSwitch];
        
        UIButton *cacheRefreshButton = [[UIButton alloc]initWithFrame:CGRectMake(selfWidth-120, cacheTxtMaxY + 10, 120, 40)];
        [cacheRefreshButton setTitle:NSLocalizedString(@"应用配置" , nil) forState:UIControlStateNormal];
        cacheRefreshButton.titleLabel.font = [UIFont systemFontOfSize:14];
        [cacheRefreshButton setTitleColor:self.tabbar.tintColor forState:UIControlStateNormal];
        [cacheRefreshButton setTitleColor:[UIColor lightGrayColor] forState:UIControlStateHighlighted];
        [cacheRefreshButton addTarget:self action:@selector(bottonButtonClick:) forControlEvents:UIControlEventTouchUpInside];
        cacheRefreshButton.tag = 2;
        [self.containView2 addSubview:cacheRefreshButton];
        self.containView2.contentSize = CGSizeMake(selfWidth, cacheRefreshButton.getMaxY+10);
    }
    return self;
}

- (UILabel *)commonLabel {
    UILabel *label = [[UILabel alloc]init];
    label.font = [UIFont systemFontOfSize:14];
    label.textColor = [UIColor blackColor];
    return label;
}

- (void)switchDidChange:(UISwitch *)mySwitch {
    if ([self.delegate respondsToSelector:@selector(settingAndConfigView:switchChangedIndex:isOpen:)]) {
        [self.delegate settingAndConfigView:self switchChangedIndex:mySwitch.tag isOpen:mySwitch.isOn];
    }
}

- (void)voiceSliderDidChange:(UISlider *)slider {
    self.muteSwitch.on = NO;
    if ([self.delegate respondsToSelector:@selector(settingAndConfigView:voiceSliderDidChange:)]) {
        [self.delegate settingAndConfigView:self voiceSliderDidChange:slider.value];
    }
}

- (void)segmentedControlDidChange:(UISegmentedControl *)segmentedControl {
    if ([self.delegate respondsToSelector:@selector(settingAndConfigView:segmentedControlIndex:selectedIndex:)]) {
        [self.delegate settingAndConfigView:self segmentedControlIndex:segmentedControl.tag selectedIndex:segmentedControl.selectedSegmentIndex];
    }
}

- (void)bottonButtonClick:(UIButton *)sender {
    if ([self.delegate respondsToSelector:@selector(settingAndConfigView:bottonButtonClickIndex:)]) {
        [self.delegate settingAndConfigView:self bottonButtonClickIndex:sender.tag];
    }
}

- (BOOL)isAccurateSeek {
    return self.accurateSeekSwitch.isOn;
}

- (BOOL)isPlayBackgournd {
    return self.playBackgournd.isOn;
}

- (void)setDataAndReloadWithArray:(NSMutableArray *)array {
    self.tableViewDataArray = array;
    [self.tableView reloadData];
}

- (void)setSelectedDataAndReloadWithArray:(NSMutableArray *)array {
    self.tableViewSelectedArray = array;
    [self.tableView reloadData];
}

- (NSArray *)getConfigArray {
    NSMutableArray *configArray = [NSMutableArray arrayWithCapacity:self.textFieldArray.count];
    for (UITextField * textField in self.textFieldArray) {
        if (textField.text.length != 0) {
            [configArray addObject:textField.text];
        }else {
            [configArray addObject:@""];
        }
    }
    if (self.showWhenStopSwitch.isOn) {
        [configArray addObject:@"1"];
    }else {
        [configArray addObject:@"0"];
    }
    return configArray.copy;
}

- (NSDictionary *)getCacheConfigDictionary {
    NSMutableDictionary *backDic = [NSMutableDictionary dictionary];
    NSArray *dicKey = @[@"maxDuration",@"maxSizeMB"];
    for (int i = 0; i<dicKey.count; i++) {
        UITextField * textField = self.cacheTextFieldArray[i];
        if (textField.text.length > 0) {
            [backDic setObject:textField.text forKey:dicKey[i]];
        }
    }
    if (self.openCacheSwitch.isOn) {
        [backDic setObject:@"1" forKey:@"enable"];
    }else {
        [backDic setObject:@"0" forKey:@"enable"];
    }
    return backDic.copy;
}

- (void)setConfigArray:(NSArray *)configArray {
    if (configArray.count != self.textFieldArray.count) { return; }
    for (int i = 0; i<self.textFieldArray.count; i++) {
        UITextField *textField = self.textFieldArray[i];
        textField.text = configArray[i];
    }
}

- (void)resetTableViewData {
    self.tableViewDataArray = @[@[],@[],@[]].mutableCopy;
    self.tableViewSelectedArray = [NSMutableArray array];
    if (self.tableView) { [self.tableView reloadData]; }
}

- (void)reloadTableView {
    [self.tableView reloadData];
}

- (void)setVolume:(CGFloat)vaule {
    self.voiceSlider.value = vaule;
}

- (void)setCurrentVideo:(NSString *)videoStr {
    self.videoStr = videoStr;
    [self.tableView reloadData];
}

- (void)setIshardwareDecoder:(BOOL)ishardwareDecoder {
    self.hardwareDecoderSwitch.on = ishardwareDecoder;
}

#pragma mark TableView

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    return self.tableViewDataArray.count+1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    if (section == self.tableViewDataArray.count) {
        return self.subtitleIndexDictionary.count;
    }
    NSArray *array = self.tableViewDataArray[section];
    return array.count;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:tableViewCellIdentifier];
    if (cell == nil) {
        cell = [[UITableViewCell alloc]initWithStyle:UITableViewCellStyleDefault reuseIdentifier:tableViewCellIdentifier];
        cell.textLabel.font = [UIFont systemFontOfSize:15];
        cell.textLabel.textAlignment = NSTextAlignmentCenter;
        cell.selectionStyle = 0;
    }
    if (indexPath.section == self.tableViewDataArray.count) {
        cell.textLabel.text = self.subtitleIndexDictionary.allKeys[indexPath.row];
        cell.accessoryType = UITableViewCellAccessoryNone;
        return cell;
    }
    NSArray *array = self.tableViewDataArray[indexPath.section];
    CicadaTrackInfo *info = array[indexPath.row];
    NSString *description = nil;
    switch (info.trackType) {
        case CICADA_TRACK_VIDEO:
            description = [NSString stringWithFormat:@"%d", info.trackBitrate];
            break;
        case CICADA_TRACK_AUDIO:
            description = (nil != info.description)? info.description : info.audioLanguage;
            break;
        case CICADA_TRACK_SUBTITLE:
            description = (nil != info.description)? info.description : info.subtitleLanguage;
            break;
        default:
            break;
    }
    cell.textLabel.text = description;
    if (indexPath.section == 0 && indexPath.row == 0) {
        if (self.isAuto) {
            cell.accessoryType = UITableViewCellAccessoryCheckmark;
        }else {
            cell.accessoryType = UITableViewCellAccessoryNone;
        }
        if (self.videoStr) {
            cell.textLabel.text = [@"AUTO(X)" stringByReplacingOccurrencesOfString:@"X" withString:self.videoStr];
        }else {
            cell.textLabel.text = @"AUTO";
        }
    }else if ([self.tableViewSelectedArray containsObject:[CicadaTool stringFromInt:info.trackIndex]]){
        cell.accessoryType = UITableViewCellAccessoryCheckmark;
    }else {
        cell.accessoryType = UITableViewCellAccessoryNone;
    }
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    if (indexPath.section == self.tableViewDataArray.count) {
        if ([self.delegate respondsToSelector:@selector(settingAndConfigView:tableViewSelectSubtitle:subtitleKey:)]) {
            UITableViewCell *cell = [tableView cellForRowAtIndexPath:indexPath];
            NSString *indexString = self.subtitleIndexDictionary[cell.textLabel.text];
            [self.delegate settingAndConfigView:self tableViewSelectSubtitle:[indexString intValue] subtitleKey:cell.textLabel.text];
        }
        return;
    }
    NSArray *array = self.tableViewDataArray[indexPath.section];
    CicadaTrackInfo *info = array[indexPath.row];
    NSString *indexString = [CicadaTool stringFromInt:info.trackIndex];
    if ([self.tableViewSelectedArray containsObject:indexString]) {
        return;
    }else {
        for (CicadaTrackInfo *eveinfo in array) {
            [self.tableViewSelectedArray removeObject:[CicadaTool stringFromInt:eveinfo.trackIndex]];
        }
        [self.tableViewSelectedArray addObject:indexString];
    }
    [tableView reloadData];
    if (info.trackIndex == -1) {
        self.isAuto = YES;
    }else if (info.trackType == CICADA_TRACK_VIDEO) {
        self.isAuto = NO;
    }
    if ([self.delegate respondsToSelector:@selector(settingAndConfigView:tableViewDidSelectTrack:)]) {
        [self.delegate settingAndConfigView:self tableViewDidSelectTrack:info];
    }
}

- (CGFloat)tableView:(UITableView *)tableView heightForHeaderInSection:(NSInteger)section {
    return 30;
}

- (UIView *)tableView:(UITableView *)tableView viewForHeaderInSection:(NSInteger)section {
    NSArray *array = @[NSLocalizedString(@"---- 清晰度 ----" , nil),NSLocalizedString(@"---- 音轨 ----" , nil),NSLocalizedString(@"---- 字幕 ----" , nil),NSLocalizedString(@"---- 外挂字幕 ----" , nil)];
    UILabel *label = [[UILabel alloc]initWithFrame:CGRectMake(0, 0, tableView.frame.size.width, 30)];
    label.font = [UIFont systemFontOfSize:15];
    label.text = array[section];
    label.backgroundColor = [UIColor colorWithWhite:0.8 alpha:1];
    label.textColor = [UIColor blackColor];
    label.textAlignment = NSTextAlignmentCenter;
    return label;
}

#pragma mark Tabbar

- (void)tabBar:(UITabBar *)tabBar didSelectItem:(UITabBarItem *)item {
    if (item.tag == self.currentIndex) { return;}
    self.currentIndex = item.tag;
    self.containView0.hidden = self.containView1.hidden = self.containView2.hidden = self.tableView.hidden = YES;
    if (item.tag == 10) {
        self.containView0.hidden = NO;
    }else if (item.tag == 11) {
        self.containView1.hidden = NO;
    }else if (item.tag == 12) {
        self.containView2.hidden = NO;
    }else if (item.tag == 13) {
        self.tableView.hidden = NO;
    }
}

@end













