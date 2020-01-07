//
//  SourceChooserDetailViewController.m
//  CicadaPlayerDemo
//
//  Created by 郦立 on 2019/1/8.
//  Copyright © 2019年 com.alibaba. All rights reserved.
//

#import "SourceChooserDetailViewController.h"
#import "CicadaPlayerViewController.h"

static NSString * const tableViewCellIdentifier = @"UITableViewCell";

@interface SourceChooserDetailViewController ()<UITableViewDelegate,UITableViewDataSource>

@property (nonatomic,strong)UITableView *tableView;

@end

@implementation SourceChooserDetailViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    self.view.backgroundColor = [UIColor whiteColor];
    self.navigationItem.title = self.model.name;
    [self addTableView];
}

- (void)addTableView {
    self.tableView = [[UITableView alloc]initWithFrame:CGRectMake(0, NAVIGATION_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT-NAVIGATION_HEIGHT-SAFE_BOTTOM)];
    self.tableView.delegate = self;
    self.tableView.dataSource = self;
    self.tableView.tableFooterView = [UIView new];
    [self.view addSubview:self.tableView];
}

#pragma mark TableView

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return self.model.samples.count;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:tableViewCellIdentifier];
    if (!cell) {
        cell = [[UITableViewCell alloc]initWithStyle:UITableViewCellStyleDefault reuseIdentifier:tableViewCellIdentifier];
        cell.textLabel.font = [UIFont systemFontOfSize:15];
        cell.textLabel.textAlignment = NSTextAlignmentCenter;
    }
    CicadaSourceSamplesModel *model = self.model.samples[indexPath.row];
    cell.textLabel.text = model.name;
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
    if (self.isClickedFlag == NO) {
        self.isClickedFlag = YES;
        CicadaSourceSamplesModel *model = self.model.samples[indexPath.row];
        if ([model.type isEqualToString:@"url"]) {
            CicadaUrlSource *source = [[CicadaUrlSource alloc] urlWithString:model.url];
            CicadaPlayerViewController *vc = [[CicadaPlayerViewController alloc]init];
            vc.subtitleDictionary = model.subtitle;
            vc.urlSource = source;
            [self.navigationController pushViewController:vc animated:YES];
        }
    }
}


@end





