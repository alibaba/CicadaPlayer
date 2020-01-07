//
//  MultChooseViewController.m
//  CicadaPlayerDemo
//
//  Created by 郦立 on 2019/6/6.
//  Copyright © 2019 com.alibaba. All rights reserved.
//

#import "MultChooseViewController.h"
#import "MultPlayerViewController.h"
#import "CicadaSourceChooserModel.h"

@interface MultChooseViewController ()<UITableViewDelegate,UITableViewDataSource>

@property (nonatomic,strong)UITableView *tableView;
@property (nonatomic,strong)NSArray *dataArray;
@property (nonatomic,strong)NSMutableArray *choosedArray;

@end

@implementation MultChooseViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    self.dataArray = [CicadaTool getSourceURLArray];
    
    self.choosedArray = [NSMutableArray array];
    self.title = NSLocalizedString(@"资源选择" , nil);
    self.tableView = [[UITableView alloc]initWithFrame:CGRectMake(0, NAVIGATION_HEIGHT, self.view.frame.size.width, self.view.frame.size.height - NAVIGATION_HEIGHT-80)];
    self.tableView.delegate = self;
    self.tableView.dataSource = self;
    self.tableView.tableFooterView = [UIView new];
    [self.view addSubview:self.tableView];
    
    UIButton *toPlayButton = [[UIButton alloc]initWithFrame:CGRectMake(20, CGRectGetMaxY(self.tableView.frame)+20, self.view.frame.size.width-40, 40)];
    toPlayButton.backgroundColor = [UIColor darkGrayColor];
    [toPlayButton setTitle:NSLocalizedString(@"进入播放" , nil) forState:UIControlStateNormal];
    [toPlayButton addTarget:self action:@selector(gotoPlay) forControlEvents:UIControlEventTouchUpInside];
    [self.view addSubview:toPlayButton];
}

- (void)gotoPlay {
    MultPlayerViewController *vc = [[MultPlayerViewController alloc]init];
    NSMutableArray *urlArray = [NSMutableArray array];
    for (CicadaSourceSamplesModel *model in self.choosedArray) {
        [urlArray addObject:model.url];
    }
    vc.playSourceArray = urlArray.copy;
    [self.navigationController pushViewController:vc animated:YES];
}

#pragma mark TableView

- (CGFloat)tableView:(UITableView *)tableView heightForRowAtIndexPath:(NSIndexPath *)indexPath {
    return 60;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return self.dataArray.count;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    static NSString *identifier = @"cell";
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:identifier];
    if (!cell) {
        cell = [[UITableViewCell alloc]initWithStyle:UITableViewCellStyleSubtitle reuseIdentifier:identifier];
        cell.selectionStyle = UITableViewCellSelectionStyleNone;
    }
    CicadaSourceSamplesModel *model = self.dataArray[indexPath.row];
    if ([self.choosedArray containsObject:model]) {
        cell.accessoryType = UITableViewCellAccessoryCheckmark;
    }else {
        cell.accessoryType = UITableViewCellAccessoryNone;
    }
    cell.textLabel.text = model.name;
    cell.detailTextLabel.text = model.url;
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    CicadaSourceSamplesModel *model = self.dataArray[indexPath.row];
    if ([self.choosedArray containsObject:model]) {
        [self.choosedArray removeObject:model];
    }else {
        [self.choosedArray addObject:model];
    }
    [tableView reloadData];
}

@end
