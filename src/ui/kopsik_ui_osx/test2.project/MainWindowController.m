//
//  MainWindowController.m
//  kopsik_ui_osx
//
//  Created by Tambet Masik on 9/24/13.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import "MainWindowController.h"
#import "LoginViewController.h"
#import "TimeEntryListViewController.h"
#import "TimerViewController.h"
#import "TimerEditViewController.h"
#import "TimeEntryViewItem.h"
#import "UIEvents.h"
#import "Context.h"

@interface MainWindowController ()
@property (nonatomic,strong) IBOutlet LoginViewController *loginViewController;
@property (nonatomic,strong) IBOutlet TimeEntryListViewController *timeEntryListViewController;
@property (nonatomic,strong) IBOutlet TimerViewController *timerViewController;
@property (nonatomic,strong) IBOutlet TimerEditViewController *timerEditViewController;
@end

@implementation MainWindowController

- (id)initWithWindow:(NSWindow *)window
{
  self = [super initWithWindow:window];
  if (self) {
    [[NSNotificationCenter defaultCenter]
      addObserver:self
      selector:@selector(eventHandler:)
      name:kUIEventUserLoggedIn
      object:nil];
    [[NSNotificationCenter defaultCenter]
     addObserver:self
     selector:@selector(eventHandler:)
     name:kUIEventUserLoggedOut
     object:nil];
    [[NSNotificationCenter defaultCenter]
     addObserver:self
     selector:@selector(eventHandler:)
     name:kUIEventTimerRunning
     object:nil];
    [[NSNotificationCenter defaultCenter]
     addObserver:self
     selector:@selector(eventHandler:)
     name:kUIEventTimerStopped
     object:nil];

    self.loginViewController = [[LoginViewController alloc]
                                initWithNibName:@"LoginViewController" bundle:nil];
    self.timeEntryListViewController = [[TimeEntryListViewController alloc]
                                        initWithNibName:@"TimeEntryListViewController" bundle:nil];
    self.timerViewController = [[TimerViewController alloc]
                                initWithNibName:@"TimerViewController" bundle:nil];
    self.timerEditViewController = [[TimerEditViewController alloc]
                                      initWithNibName:@"TimerEditViewController" bundle:nil];
    
    [self.loginViewController.view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [self.timerViewController.view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [self.timerEditViewController.view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [self.timeEntryListViewController.view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
  }
  return self;
}

- (void)windowDidLoad
{
  [super windowDidLoad];
    
  char err[KOPSIK_ERR_LEN];
  TogglUser *user = kopsik_user_init();
  if (KOPSIK_API_SUCCESS != kopsik_current_user(ctx, err, KOPSIK_ERR_LEN, user)) {
    NSLog(@"Error fetching user: %s", err);
  } else if (!user->ID) {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventUserLoggedOut object:nil];
  } else {
    NSLog(@"Current user: %s", user->Fullname);
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventUserLoggedIn object:nil];
    
    // Get running time entry
    TogglTimeEntryViewItem *item = kopsik_time_entry_view_item_init();
    int is_tracking = 0;
    if (KOPSIK_API_SUCCESS == kopsik_running_time_entry_view_item(ctx,
                                                                  err, KOPSIK_ERR_LEN,
                                                                  item, &is_tracking)) {
      if (is_tracking) {
        TimeEntryViewItem *te = [[TimeEntryViewItem alloc] init];
        [te load:item];
        [[NSNotificationCenter defaultCenter]
         postNotificationName:kUIEventTimerRunning object:te];
      } else {
        [[NSNotificationCenter defaultCenter]
         postNotificationName:kUIEventTimerStopped object:nil];
      }
    } else {
      NSLog(@"Error fetching running time entry: %s", err);
    }
    kopsik_time_entry_view_item_clear(item);
  }
  kopsik_user_clear(user);
}

-(void)eventHandler: (NSNotification *) notification
{
  NSLog(@"event triggered: %@", notification.name);
  if ([notification.name isEqualToString:kUIEventUserLoggedIn]) {
    // Hide login view
    [self.loginViewController.view removeFromSuperview];

    // Show time entry list
    [self.contentView addSubview:self.timeEntryListViewController.view];
    [self.timeEntryListViewController.view setFrame:self.contentView.bounds];
    
    // Show footer
    [self.footerView setHidden:NO];
    
  } else if ([notification.name isEqualToString:kUIEventUserLoggedOut]) {
    // Show login view
    [self.contentView addSubview:self.loginViewController.view];
    [self.loginViewController.view setFrame:self.contentView.bounds];

    // Hide all other views
    [self.timeEntryListViewController.view removeFromSuperview];
    [self.footerView setHidden:YES];
    [self.timeEntryListViewController.view removeFromSuperview];

  } else if ([notification.name isEqualToString:kUIEventTimerRunning]) {
    [self.headerView addSubview:self.timerViewController.view];
    [self.timerViewController.view setFrame: self.headerView.bounds];
    
    [self.timerEditViewController.view removeFromSuperview];
    
  } else if ([notification.name isEqualToString:kUIEventTimerStopped]) {
    [self.timerViewController.view removeFromSuperview];
    [self.headerView addSubview:self.timerEditViewController.view];
    [self.timerEditViewController.view setFrame:self.headerView.bounds];
  }
}

- (IBAction)logout:(id)sender {
  char err[KOPSIK_ERR_LEN];
  if (KOPSIK_API_SUCCESS != kopsik_logout(ctx, err, KOPSIK_ERR_LEN)) {
    NSLog(@"Logout error: %s", err);
    return;
  }
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventUserLoggedOut object:nil];
}

- (IBAction)sync:(id)sender {
  char err[KOPSIK_ERR_LEN];
  if (KOPSIK_API_SUCCESS != kopsik_sync(ctx, err, KOPSIK_ERR_LEN)) {
    NSLog(@"Sync error: %s", err);
    return;
  }
}

@end
