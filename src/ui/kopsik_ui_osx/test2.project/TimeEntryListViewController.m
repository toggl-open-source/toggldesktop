//
//  TimeEntryListViewController.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 Alari. All rights reserved.
//

#import "TimeEntryListViewController.h"
#import "UIEvents.h"
#import "kopsik_api.h"

@interface TimeEntryListViewController ()

@end

@implementation TimeEntryListViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
      [[NSNotificationCenter defaultCenter]
       addObserver:self
       selector:@selector(eventHandler:)
       name:kUIEventUserLoggedIn
       object:nil];
    }
    return self;
}

-(void)eventHandler: (NSNotification *) notification
{
  if ([notification.name isEqualToString:kUIEventUserLoggedIn]) {
    char err[KOPSIK_ERR_LEN];
    TogglTimeEntryList *time_entries = kopsik_time_entry_list_new();
    if (KOPSIK_API_SUCCESS != kopsik_time_entries(err, KOPSIK_ERR_LEN, time_entries)) {
      NSLog(@"Error fetching time entries: %s", err);
    } else {
      // FIXME: render entries
    }
    kopsik_time_entry_list_delete(time_entries);
  }
}

@end
