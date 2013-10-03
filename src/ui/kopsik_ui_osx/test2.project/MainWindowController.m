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
#import "TimeEntryEditViewController.h"
#import "TimeEntryViewItem.h"
#import "UIEvents.h"
#import "Context.h"
#import "Bugsnag.h"
#import "User.h"

@interface MainWindowController ()
@property (nonatomic,strong) IBOutlet LoginViewController *loginViewController;
@property (nonatomic,strong) IBOutlet TimeEntryListViewController *timeEntryListViewController;
@property (nonatomic,strong) IBOutlet TimerViewController *timerViewController;
@property (nonatomic,strong) IBOutlet TimerEditViewController *timerEditViewController;
@property (nonatomic,strong) IBOutlet TimeEntryEditViewController *timeEntryEditViewController;
@end

@implementation MainWindowController

- (id)initWithWindow:(NSWindow *)window
{
  self = [super initWithWindow:window];
  if (self) {
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(eventHandler:)
                                                 name:kUIEventUserLoggedIn
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(eventHandler:)
                                                 name:kUIEventUserLoggedOut
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(eventHandler:)
                                                 name:kUIEventTimerRunning
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(eventHandler:)
                                                 name:kUIEventTimerStopped
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(eventHandler:)
                                                 name:kUIEventTimeEntrySelected
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(eventHandler:)
                                                 name:kUIEventTimeEntryDeselected
                                               object:nil];

    self.loginViewController = [[LoginViewController alloc]
                                initWithNibName:@"LoginViewController" bundle:nil];
    self.timeEntryListViewController = [[TimeEntryListViewController alloc]
                                        initWithNibName:@"TimeEntryListViewController" bundle:nil];
    self.timerViewController = [[TimerViewController alloc]
                                initWithNibName:@"TimerViewController" bundle:nil];
    self.timerEditViewController = [[TimerEditViewController alloc]
                                      initWithNibName:@"TimerEditViewController" bundle:nil];
    self.timeEntryEditViewController = [[TimeEntryEditViewController alloc]
                                    initWithNibName:@"TimeEntryEditViewController" bundle:nil];
    
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
  KopsikUser *user = kopsik_user_init();
  if (KOPSIK_API_SUCCESS != kopsik_current_user(ctx, err, KOPSIK_ERR_LEN, user)) {
    NSLog(@"Error fetching user: %s", err);
    [Bugsnag notify:[NSException
                     exceptionWithName:@"Error fetching user"
                     reason:[NSString stringWithUTF8String:err]
                     userInfo:nil]];
    kopsik_user_clear(user);
    return;
  }

  User *userinfo = nil;
  if (user->ID) {
    userinfo = [[User alloc] init];
    [userinfo load:user];
  }
  kopsik_user_clear(user);
  
  if (userinfo == nil) {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventUserLoggedOut object:nil];
  } else {
    NSLog(@"Current user: %s", user->Fullname);
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventUserLoggedIn object:userinfo];
  }
}

-(void)eventHandler: (NSNotification *) notification
{
  NSLog(@"event triggered: %@", notification.name);
  if ([notification.name isEqualToString:kUIEventUserLoggedIn]) {
    User *userinfo = notification.object;
    [Bugsnag setUserAttribute:@"user_id" withValue:[NSString stringWithFormat:@"%ld", userinfo.ID]];
    
    // Hide login view
    [self.loginViewController.view removeFromSuperview];

    // Show time entry list
    [self.contentView addSubview:self.timeEntryListViewController.view];
    [self.timeEntryListViewController.view setFrame:self.contentView.bounds];
    
    // Show header and footer
    [self.footerView setHidden:NO];
    [self.headerView setHidden:NO];
    
    renderRunningTimeEntry();
    
    [self startSync];
    
  } else if ([notification.name isEqualToString:kUIEventUserLoggedOut]) {
    [Bugsnag setUserAttribute:@"user_id" withValue:nil];
    
    // Show login view
    [self.contentView addSubview:self.loginViewController.view];
    [self.loginViewController.view setFrame:self.contentView.bounds];
    [self.loginViewController.email becomeFirstResponder];

    // Hide all other views
    [self.timeEntryListViewController.view removeFromSuperview];
    [self.footerView setHidden:YES];
    [self.headerView setHidden:YES];
    [self.timerViewController.view removeFromSuperview];

  } else if ([notification.name isEqualToString:kUIEventTimerRunning]) {
    [self.headerView addSubview:self.timerViewController.view];
    [self.timerViewController.view setFrame: self.headerView.bounds];
    
    [self.timerEditViewController.view removeFromSuperview];
    
  } else if ([notification.name isEqualToString:kUIEventTimerStopped]) {
    [self.timerViewController.view removeFromSuperview];

    [self.headerView addSubview:self.timerEditViewController.view];
    [self.timerEditViewController.view setFrame:self.headerView.bounds];

  } else if ([notification.name isEqualToString:kUIEventTimeEntrySelected]) {
    [self.headerView setHidden:YES];
    [self.timeEntryListViewController.view removeFromSuperview];
    [self.contentView addSubview:self.timeEntryEditViewController.view];
    [self.timeEntryEditViewController.view setFrame:self.contentView.bounds];

  } else if ([notification.name isEqualToString:kUIEventTimeEntryDeselected]) {
    [self.headerView setHidden:NO];
    [self.timeEntryEditViewController.view removeFromSuperview];
    [self.contentView addSubview:self.timeEntryListViewController.view];
    [self.timeEntryListViewController.view setFrame:self.contentView.bounds];
  }
}

- (IBAction)logout:(id)sender {
  char err[KOPSIK_ERR_LEN];
  if (KOPSIK_API_SUCCESS != kopsik_logout(ctx, err, KOPSIK_ERR_LEN)) {
    NSLog(@"Logout error: %s", err);
    [Bugsnag notify:[NSException
                     exceptionWithName:@"Logout error"
                     reason:[NSString stringWithUTF8String:err]
                     userInfo:nil]];
    return;
  }
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventUserLoggedOut object:nil];
}

- (IBAction)sync:(id)sender {
  [self startSync];
}

void finishSync(kopsik_api_result result, char *err, unsigned int errlen) {
  NSLog(@"finishSync");
  if (KOPSIK_API_SUCCESS != result) {
    NSLog(@"finishSync error %s", err);
    [Bugsnag notify:[NSException
                     exceptionWithName:@"finishSync error"
                     reason:[NSString stringWithUTF8String:err]
                     userInfo:nil]];
    free(err);
    return;
  }
  renderRunningTimeEntry();
}

void renderRunningTimeEntry() {
  KopsikTimeEntryViewItem *item = kopsik_time_entry_view_item_init();
  int is_tracking = 0;
  char err[KOPSIK_ERR_LEN];
  if (KOPSIK_API_SUCCESS != kopsik_running_time_entry_view_item(ctx,
                                                                err, KOPSIK_ERR_LEN,
                                                                item, &is_tracking)) {
    NSLog(@"Error fetching running time entry: %s", err);
    kopsik_time_entry_view_item_clear(item);
    [Bugsnag notify:[NSException
                     exceptionWithName:@"Error fetching running time entry"
                     reason:[NSString stringWithUTF8String:err]
                     userInfo:nil]];
    return;
  }

  if (is_tracking) {
    TimeEntryViewItem *te = [[TimeEntryViewItem alloc] init];
    [te load:item];
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventTimerRunning object:te];
  } else {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventTimerStopped object:nil];
  }
  kopsik_time_entry_view_item_clear(item);
}

- (void)startSync {
  kopsik_sync_async(ctx, 1, finishSync);
}

@end
