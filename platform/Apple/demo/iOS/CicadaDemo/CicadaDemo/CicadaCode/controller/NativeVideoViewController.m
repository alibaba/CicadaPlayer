//
//  NativeVideoViewController.m
//  CicadaPlayerDemo
//
//  Created by 郦立 on 2019/1/13.
//  Copyright © 2019年 com.alibaba. All rights reserved.
//

#import "NativeVideoViewController.h"
#import "CicadaPlayerViewController.h"

static NSString * const tableViewCellIdentifier = @"UITableViewCell";

@interface NativeVideoViewController ()<UITableViewDelegate,UITableViewDataSource>

@property (weak, nonatomic) IBOutlet UITableView *tableview;
@property (nonatomic,strong)NSArray *dataArray;

@end

@implementation NativeVideoViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    self.tableview.tableFooterView = [UIView new];
    
    self.dataArray = [CicadaTool getDocumentMP4Array];
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
    }
    NSString *filePath = self.dataArray[indexPath.row];
    NSArray *sepArray = [filePath componentsSeparatedByString:@"/"];
    cell.textLabel.text = sepArray.lastObject;
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    [tableView deselectRowAtIndexPath:indexPath animated:YES];
    if (self.isClickedFlag == NO) {
        self.isClickedFlag = YES;
        NSString *filePath = self.dataArray[indexPath.row];
        CicadaUrlSource *source = [[CicadaUrlSource alloc] urlWithString:filePath];
        CicadaPlayerViewController *vc = [[CicadaPlayerViewController alloc]init];
        vc.urlSource = source;
        [self.navigationController pushViewController:vc animated:YES];
    }
}

@end





