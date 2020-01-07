//
//  SourceChooserViewController.m
//  CicadaPlayerDemo
//
//  Created by 郦立 on 2018/12/28.
//  Copyright © 2018年 com.alibaba. All rights reserved.
//

#import "SourceChooserViewController.h"
#import "CicadaSourceChooserModel.h"
#import "SourceChooserDetailViewController.h"
#import "MultChooseViewController.h"

static NSString * const tableViewCellIdentifier = @"UITableViewCell";

@interface SourceChooserViewController ()<UITableViewDelegate,UITableViewDataSource>

@property (weak, nonatomic) IBOutlet UITableView *mainTableView;
@property (nonatomic,strong)NSArray *dataArray;

@end

@implementation SourceChooserViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    self.dataArray = [CicadaTool getSourceSamplesArray];
    self.mainTableView.tableFooterView = [UIView new];
}

- (IBAction)gotoMultPlayer:(id)sender {
    if (self.isClickedFlag == NO) {
        self.isClickedFlag = YES;
        MultChooseViewController *vc = [[MultChooseViewController alloc]init];
        [self.navigationController pushViewController:vc animated:YES];
    }
}

#pragma mark storyBoard

- (BOOL)shouldPerformSegueWithIdentifier:(NSString *)identifier sender:(id)sender {
    if (self.isClickedFlag == NO) {
        self.isClickedFlag = YES;
        return YES;
    }
    return NO;
}

#pragma mark TableView

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return self.dataArray.count;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:tableViewCellIdentifier];
    if (!cell) {
        cell = [[UITableViewCell alloc]initWithStyle:UITableViewCellStyleDefault reuseIdentifier:tableViewCellIdentifier];
        cell.textLabel.font = [UIFont systemFontOfSize:15];
        cell.textLabel.textAlignment = NSTextAlignmentCenter;
        cell.backgroundColor = [UIColor lightGrayColor];
        cell.textLabel.textColor = [UIColor blackColor];
    }
    CicadaSourceChooserModel *chooserModel = self.dataArray[indexPath.row];
    cell.textLabel.text = chooserModel.name;
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
    if (self.isClickedFlag == NO) {
        self.isClickedFlag = YES;
        SourceChooserDetailViewController *vc = [[SourceChooserDetailViewController alloc]init];
        CicadaSourceChooserModel *model = self.dataArray[indexPath.row];
        vc.model = model;
        [self.navigationController pushViewController:vc animated:YES];
    }
}

@end






