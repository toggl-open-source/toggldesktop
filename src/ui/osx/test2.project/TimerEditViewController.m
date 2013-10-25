//
//  TimerEditViewController.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import "TimerEditViewController.h"
#import "UIEvents.h"

@interface TimerEditViewController ()

@end

@implementation TimerEditViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Initialization code here.
    }
    
    return self;
}

- (IBAction)startButtonClicked:(id)sender {
  NSString *description = [self.descriptionTextField stringValue];
  [[NSNotificationCenter defaultCenter] postNotificationName:kUICommandNew
                                                      object:description];
  [self.descriptionTextField setStringValue:@""];
  
}

@end
