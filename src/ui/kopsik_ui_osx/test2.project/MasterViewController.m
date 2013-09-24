//
//  MasterViewController.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 Alari. All rights reserved.
//

#import "MasterViewController.h"

#include "FooterViewController.h"
#include "HeaderViewController.h"
#include "ContentViewController.h"

@interface MasterViewController ()
@property (nonatomic,strong) IBOutlet HeaderViewController *headerViewController;
@property (nonatomic,strong) IBOutlet ContentViewController *contentViewController;
@property (nonatomic,strong) IBOutlet FooterViewController *footerViewController;
@end

@implementation MasterViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        self.contentViewController = [[ContentViewController alloc] initWithNibName:@"ContentViewController" bundle:nil];
        self.headerViewController = [[HeaderViewController alloc] init];
        self.footerViewController = [[FooterViewController alloc] init];
    }
    
    return self;
}

@end
