//
//  RightView.m
//  AliPlayerDemo
//
//  Created by 汪宁 on 2019/2/15.
//  Copyright © 2019年 com.alibaba. All rights reserved.
//

#import "RightView.h"
#import "AliPlayerViewController.h"
#import "AppDelegate.h"
#import "CicadaSourceChooserModel.h"
#import "MutlPlayViewController.h"

@interface RightView()<NSCollectionViewDelegate,NSCollectionViewDataSource,NSTableViewDelegate, NSTableViewDataSource>

// url 播放
@property (strong) IBOutlet NSTextField *urlTextField;

@property (strong) IBOutlet NSTextField *localTextField;

// url Links
@property (nonatomic, strong) NSArray *tableViewArray;
@property (nonatomic, strong) CicadaSourceSamplesModel *model;
@property (nonatomic, strong) CicadaSourceChooserModel *chooserModel;
@property (nonatomic, strong) NSView *view;
@property (nonatomic, assign) ViewType type;

//多画面播放
@property (weak) IBOutlet NSTextField *multSourceText1;
@property (weak) IBOutlet NSTextField *multSourceText2;
@property (weak) IBOutlet NSTextField *multSourceText3;
@property (weak) IBOutlet NSTextField *multSourceText4;

@end

@implementation RightView

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];
    
    // Drawing code here.
}

- (instancetype)initWithCoder:(NSCoder *)aDecoder {
    self = [super initWithCoder:aDecoder];
    if (self) {
        self.tableViewArray = [[NSArray alloc]init];
        _view = nil;
        NSNib *xib = [[NSNib alloc] initWithNibNamed:@"RightView" bundle:nil];
        NSArray *viewsArray = [[NSArray alloc] init];
        
        [xib instantiateWithOwner:self topLevelObjects:&viewsArray];
        for (int i = 0; i < viewsArray.count; i++) {
            if ([viewsArray[i] isKindOfClass:[NSView class]]) {
                _view = viewsArray[i];
                if ([_view.identifier isEqualToString:@"inputURL"]) {
                    break;
                }
               
            }
        }

        [self addSubview:_view];
    }
    return self;
}

- (void)loadViewWithViewType:(ViewType)viewType {
    self.type = viewType;
    NSString *ID = @"";
    switch (viewType) {
        case inputURL:{
            ID = @"inputURL";
        }
            break;
        case MultPlayer: {
            ID = @"MultSource";
        }
            break;
        case urlLinks: {
            ID = @"urlLinks";
            self.chooserModel = [[HudTool getSourceSamplesArray]objectAtIndex:0];
            self.tableViewArray = [HudTool getDocumentMP4Array];
        }
            break;
        case localURL: {
            ID = @"localURL";
        }
            break;
        default:
            break;
    }
    [_view removeFromSuperview];
    _view = nil;
    NSNib *xib = [[NSNib alloc] initWithNibNamed:@"RightView" bundle:nil];
    NSArray *viewsArray = [[NSArray alloc] init];
    
    [xib instantiateWithOwner:self topLevelObjects:&viewsArray];
    for (int i = 0; i < viewsArray.count; i++) {
        if ([viewsArray[i] isKindOfClass:[NSView class]]) {
            _view = viewsArray[i];
            if ([_view.identifier isEqualToString:ID]) {
                break;
            }
        }
    }
  
    [self addSubview:_view];
    if (viewType == MultPlayer) {
        for (int i = 1; i<5; i++) {
            NSTextField *textField = [_view viewWithTag:i];
            NSString *key = [NSString stringWithFormat:@"multSourceText%d",i];
            NSString *value = [[NSUserDefaults standardUserDefaults] valueForKey:key];
            if (value) {
                textField.stringValue = value;
            }
        }
    }
}

- (IBAction)chooseFile:(id)sender {
    
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];
    [openPanel setPrompt: @"打开"];
    
    openPanel.allowedFileTypes = [NSArray arrayWithObjects: @"mp3", @"mp4",@"m3u8",@"mov",@"flv",nil];
    openPanel.directoryURL = nil;
    
    NSWindow *window =  [NSApplication sharedApplication].keyWindow;
    
   __block typeof(self) weakSelf = self;
    [openPanel beginSheetModalForWindow:window completionHandler:^(NSModalResponse returnCode) {
        
        if (returnCode == 1) {
            NSURL *fileUrl = [[openPanel URLs] objectAtIndex:0];
            NSString * file = [[fileUrl absoluteString]substringFromIndex:7];
            weakSelf.localTextField.stringValue = file;
        }
    }];
    
}

- (IBAction)enterURLPlayer:(id)sender {
    
    if (self.urlTextField.stringValue.length == 0) {
        [HudTool hudWithText:@"数据不全"];
        return;
    }
    
    NSString *identifier = @"pushWindow";
    NSString *title = @"播放";

    NSWindowController * windowVC = [[NSStoryboard storyboardWithName:@"Main" bundle:nil]instantiateControllerWithIdentifier:identifier];
    windowVC.window.title = title;
    NSString *urlStr = self.urlTextField.stringValue;
    
    CicadaUrlSource *source = [[CicadaUrlSource alloc] urlWithString:urlStr];
    [windowVC.contentViewController setValue:source forKey:@"urlSource"];
    
    AppDelegate *appdelegate = (AppDelegate *) [NSApplication sharedApplication].delegate;
    [appdelegate.mainWindow.window close];
    appdelegate.mainWindow = windowVC;
    [windowVC.window setContentSize:CGSizeMake(900, 500)];
    [windowVC.window setContentMinSize:CGSizeMake(900, 500)];
    [windowVC.window orderFront:nil];
}

- (IBAction)localUrlPlay:(id)sender {
    if (_localTextField.stringValue.length == 0) {
        [HudTool hudWithText:@"数据不全"];
        return;
    }
   
    NSWindowController * windowVC = [[NSStoryboard storyboardWithName:@"Main" bundle:nil]instantiateControllerWithIdentifier:@"pushWindow"];
    windowVC.window.title = @"播放";
    
    NSString *dataGBK = [_localTextField.stringValue stringByReplacingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
    CicadaUrlSource *source = [[CicadaUrlSource alloc] urlWithString:dataGBK];
    AliPlayerViewController *VC = (AliPlayerViewController *)windowVC.contentViewController;
    VC.urlSource = source;
    AppDelegate *appdelegate = (AppDelegate *) [NSApplication sharedApplication].delegate;
    [appdelegate.mainWindow.window close];
    appdelegate.mainWindow = windowVC;
    [windowVC.window setContentSize:CGSizeMake(900, 500)];
    [windowVC.window setContentMinSize:CGSizeMake(900, 500)];
    [windowVC.window orderFront:nil];
}

- (void)reloadCollectionViewWithArray:(NSArray *)array{

}

- (IBAction)enterMultSourcePlay:(id)sender {
    
    if ((self.multSourceText1.stringValue.length == 0) || (self.multSourceText2.stringValue.length == 0) ||(self.multSourceText3.stringValue.length == 0) ||(self.multSourceText4.stringValue.length == 0)) {
        [HudTool hudWithText:@"数据不全"];
        return;
    }
    
    NSWindowController * windowVC = [[NSStoryboard storyboardWithName:@"Main" bundle:nil]instantiateControllerWithIdentifier:@"MultWindow"];
    windowVC.window.title = @"多画面播放";
        
    MutlPlayViewController *VC = (MutlPlayViewController *)windowVC.contentViewController;
    NSTextField *textField1 = [_view viewWithTag:1];
    NSTextField *textField2 = [_view viewWithTag:2];
    NSTextField *textField3 = [_view viewWithTag:3];
    NSTextField *textField4 = [_view viewWithTag:4];
    CicadaUrlSource *source1 = [[CicadaUrlSource alloc] urlWithString:textField1.stringValue];
    CicadaUrlSource *source2 = [[CicadaUrlSource alloc] urlWithString:textField2.stringValue];
    CicadaUrlSource *source3 = [[CicadaUrlSource alloc] urlWithString:textField3.stringValue];
    CicadaUrlSource *source4 = [[CicadaUrlSource alloc] urlWithString:textField4.stringValue];
    VC.urlSourceArray = @[source1,source2,source3,source4];
    
    [[NSUserDefaults standardUserDefaults]setObject:self.multSourceText1.stringValue forKey:@"multSourceText1"];
    [[NSUserDefaults standardUserDefaults]setObject:self.multSourceText2.stringValue forKey:@"multSourceText2"];
    [[NSUserDefaults standardUserDefaults]setObject:self.multSourceText3.stringValue forKey:@"multSourceText3"];
    [[NSUserDefaults standardUserDefaults]setObject:self.multSourceText4.stringValue forKey:@"multSourceText4"];
    
    AppDelegate *appdelegate = (AppDelegate *) [NSApplication sharedApplication].delegate;
    [appdelegate.mainWindow.window close];
    appdelegate.mainWindow = windowVC;
    [windowVC.window setContentSize:CGSizeMake(900, 500)];
    [windowVC.window setContentMinSize:CGSizeMake(900, 500)];
    [windowVC.window orderFront:nil];
    
}

#pragma mark collectionView 代理函数

- (NSInteger)collectionView:(NSCollectionView *)collectionView numberOfItemsInSection:(NSInteger)section {
    return 1;
}

- (NSInteger)numberOfSectionsInCollectionView:(NSCollectionView *)collectionView {
    return 1;
}


- (NSCollectionViewItem *)collectionView:(NSCollectionView *)collectionView itemForRepresentedObjectAtIndexPath:(NSIndexPath *)indexPath {
    return nil;
}


- (void)collectionView:(NSCollectionView *)collectionView didSelectItemsAtIndexPaths:(NSSet<NSIndexPath *> *)indexPaths {
}

#pragma mark tableView 代理函数

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView {
    return _chooserModel.samples.count;
   // return _tableViewArray.count;
}


- (void)tableViewSelectionDidChange:(NSNotification *)notification{

    NSTableView *tableView = notification.object;
    
    NSInteger row =   tableView.selectedRow;
    _model = _chooserModel.samples[row];
    NSWindowController * windowVC = [[NSStoryboard storyboardWithName:@"Main" bundle:nil]instantiateControllerWithIdentifier:@"pushWindow"];
    windowVC.window.title = @"播放";
    CicadaUrlSource *urlSource = [[CicadaUrlSource alloc] urlWithString:_model.url];

    AliPlayerViewController *VC = (AliPlayerViewController *)windowVC.contentViewController;
    VC.urlSource = urlSource;
    AppDelegate *appdelegate = (AppDelegate *) [NSApplication sharedApplication].delegate;
    [appdelegate.mainWindow.window close];
    appdelegate.mainWindow.window = nil;
    appdelegate.mainWindow = windowVC;
    [windowVC.window orderFront:nil];
    [tableView deselectRow:row];
}


-(CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row{
    return 44;
}


- (nullable NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(nullable NSTableColumn *)tableColumn row:(NSInteger)row {
    
    NSString *strIdt = @"tableViewCell";

    NSTableCellView *cell = [tableView makeViewWithIdentifier:strIdt owner:self];
    if (!cell) {
        cell = [[NSTableCellView alloc]init];
        cell.identifier = strIdt;
    }
    _model = _chooserModel.samples[row];
    cell.textField.stringValue = _model.name;
    //cell.textField.stringValue = [_tableViewArray objectAtIndex:row];
    return cell;
}

-(void)layout{
    [super layout];
    _view.frame = self.bounds;
    NSCollectionViewFlowLayout *flowLayout = [[NSCollectionViewFlowLayout alloc]init];
    flowLayout.sectionInset = NSEdgeInsetsMake(10, 5, 10, 5);
    flowLayout.minimumInteritemSpacing = 5;
    flowLayout.minimumLineSpacing = 5;
    CGFloat width = (self.bounds.size.width - 60)/6;
    flowLayout.itemSize = CGSizeMake(width, width*1.1);
}


-(void)touchesBeganWithEvent:(NSEvent *)event{
    NSLog(@"scroll view click");
}

@end
