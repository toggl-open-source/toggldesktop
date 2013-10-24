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
#import "ModelChange.h"
#import "ErrorHandler.h"

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
    [self.timeEntryEditViewController.view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(eventHandler:)
                                                 name:kUIStateUserLoggedIn
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(eventHandler:)
                                                 name:kUIStateUserLoggedOut
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(eventHandler:)
                                                 name:kUIStateTimerRunning
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(eventHandler:)
                                                 name:kUIStateTimerStopped
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(eventHandler:)
                                                 name:kUIStateTimeEntrySelected
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(eventHandler:)
                                                 name:kUIStateTimeEntryDeselected
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(eventHandler:)
                                                 name:kUIStateError
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(eventHandler:)
                                                 name:kUIEventModelChange
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(eventHandler:)
                                                 name:kUICommandNew
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(eventHandler:)
                                                 name:kUICommandContinue
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(eventHandler:)
                                                 name:kUICommandStop
                                               object:nil];
  }
  return self;
}

- (void)windowDidLoad
{
  [super windowDidLoad];
  
  NSLog(@"MainWindow windowDidLoad");
  
  kopsik_set_change_callback(ctx, on_model_change);
  
  char err[KOPSIK_ERR_LEN];
  KopsikUser *user = kopsik_user_init();
  if (KOPSIK_API_SUCCESS != kopsik_current_user(ctx, err, KOPSIK_ERR_LEN, user)) {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateError
                                                        object:[NSString stringWithUTF8String:err]];
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
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateUserLoggedOut object:nil];
  } else {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateUserLoggedIn object:userinfo];
  }
}

-(void)eventHandler: (NSNotification *) notification
{
  NSLog(@"osx_ui.%@ %@", notification.name, notification.object);

  if ([notification.name isEqualToString:kUIStateUserLoggedIn]) {
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
    
    [self performSelector:@selector(startSync) withObject:nil afterDelay:0.5];
    [self performSelector:@selector(startWebSocket) withObject:nil afterDelay:0.5];

  } else if ([notification.name isEqualToString:kUIStateUserLoggedOut]) {
    [Bugsnag setUserAttribute:@"user_id" withValue:nil];
    
    NSLog(@"MainWindow stopping websocket");
    
    kopsik_websocket_stop_async(ctx, handle_error);

    // Show login view
    [self.contentView addSubview:self.loginViewController.view];
    [self.loginViewController.view setFrame:self.contentView.bounds];
    [self.loginViewController.email becomeFirstResponder];

    // Hide all other views
    [self.timeEntryListViewController.view removeFromSuperview];
    [self.footerView setHidden:YES];
    [self.headerView setHidden:YES];
    [self.timerViewController.view removeFromSuperview];

  } else if ([notification.name isEqualToString:kUIStateTimerRunning]) {
    // Hide timer editor from header view
    [self.timerEditViewController.view removeFromSuperview];
    
    // If running timer view is not visible yet, add it to header view
    for (int i = 0; i < [self.headerView subviews].count; i++) {
      if ([[self.headerView subviews] objectAtIndex:i] == self.timerViewController.view) {
        return;
      }
    }
    [self.headerView addSubview:self.timerViewController.view];
    [self.timerViewController.view setFrame: self.headerView.bounds];
    
  } else if ([notification.name isEqualToString:kUIStateTimerStopped]) {
    // Hide running timer view from header view
    [self.timerViewController.view removeFromSuperview];
    
    // If timer editor is not visible yet, add it to header view
    for (int i = 0; i < [self.headerView subviews].count; i++) {
      if ([[self.headerView subviews] objectAtIndex:i] == self.timerEditViewController.view) {
        return;
      }
    }
    [self.headerView addSubview:self.timerEditViewController.view];
    [self.timerEditViewController.view setFrame:self.headerView.bounds];

  } else if ([notification.name isEqualToString:kUIStateTimeEntrySelected]) {
    [self.headerView setHidden:YES];
    [self.timeEntryListViewController.view removeFromSuperview];
    [self.contentView addSubview:self.timeEntryEditViewController.view];
    [self.timeEntryEditViewController.view setFrame:self.contentView.bounds];

  } else if ([notification.name isEqualToString:kUIStateTimeEntryDeselected]) {
    [self.headerView setHidden:NO];
    [self.timeEntryEditViewController.view removeFromSuperview];
    [self.contentView addSubview:self.timeEntryListViewController.view];
    [self.timeEntryListViewController.view setFrame:self.contentView.bounds];
  
  } else if ([notification.name isEqualToString:kUICommandNew]) {
    NSString *description = notification.object;
    char err[KOPSIK_ERR_LEN];
    KopsikTimeEntryViewItem *item = kopsik_time_entry_view_item_init();
    if (KOPSIK_API_SUCCESS != kopsik_start(ctx, err, KOPSIK_ERR_LEN, [description UTF8String], item)) {
      kopsik_time_entry_view_item_clear(item);
      [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateError
                                                          object:[NSString stringWithUTF8String:err]];
      return;
    }
    
    TimeEntryViewItem *te = [[TimeEntryViewItem alloc] init];
    [te load:item];
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateTimerRunning object:te];
    
    kopsik_push_async(ctx, handle_error);
    
  } else if ([notification.name isEqualToString:kUICommandContinue]) {
    NSString *guid = notification.object;
    char err[KOPSIK_ERR_LEN];
    KopsikTimeEntryViewItem *item = kopsik_time_entry_view_item_init();
    kopsik_api_result res = 0;
    int was_found = 0;
    if (guid == nil) {
      res = kopsik_continue_latest(ctx, err, KOPSIK_ERR_LEN, item, &was_found);
    } else {
      was_found = 1;
      res = kopsik_continue(ctx, err, KOPSIK_ERR_LEN, [guid UTF8String], item);
    }

    if (res != KOPSIK_API_SUCCESS) {
      kopsik_time_entry_view_item_clear(item);
      [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateError
                                                          object:[NSString stringWithUTF8String:err]];
      return;
    }
    
    if (!was_found) {
      kopsik_time_entry_view_item_clear(item);
      return;
    }
    
    TimeEntryViewItem *te = [[TimeEntryViewItem alloc] init];
    [te load:item];
    kopsik_time_entry_view_item_clear(item);
    
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateTimerRunning object:te];
    
    kopsik_push_async(ctx, handle_error);
  
  } else if ([notification.name isEqualToString:kUICommandStop]) {
    char err[KOPSIK_ERR_LEN];
    KopsikTimeEntryViewItem *item = kopsik_time_entry_view_item_init();
    int was_found = 0;
    if (KOPSIK_API_SUCCESS != kopsik_stop(ctx, err, KOPSIK_ERR_LEN, item, &was_found)) {
      kopsik_time_entry_view_item_clear(item);
      [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateError
                                                          object:[NSString stringWithUTF8String:err]];
      return;
    }
    
    if (!was_found) {
      kopsik_time_entry_view_item_clear(item);
      return;
    }

    TimeEntryViewItem *te = [[TimeEntryViewItem alloc] init];
    [te load:item];
    kopsik_time_entry_view_item_clear(item);
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateTimerStopped object:te];
    
    kopsik_push_async(ctx, handle_error);

  } else if ([notification.name isEqualToString:kUIStateError]) {
    // Proxy all app errors through this notification.

    NSString *msg = notification.object;
    NSLog(@"Error: %@", msg);

    // Ignore offline errors
    if ([msg rangeOfString:@"Host not found"].location != NSNotFound) {
      return;
    }

    [self performSelectorOnMainThread:@selector(showError:) withObject:msg waitUntilDone:NO];

    [Bugsnag notify:[NSException
                     exceptionWithName:@"UI error"
                     reason:msg
                     userInfo:nil]];
  }
}

- (void)showError:(NSString *)msg {
  NSAlert *alert = [[NSAlert alloc] init];
  [alert setMessageText:msg];
  [alert addButtonWithTitle:@"Dismiss"];
  [alert beginSheetModalForWindow:self.window
                    modalDelegate:self
                   didEndSelector:nil
                      contextInfo:nil];
}

- (IBAction)logout:(id)sender {
  char err[KOPSIK_ERR_LEN];
  if (KOPSIK_API_SUCCESS != kopsik_logout(ctx, err, KOPSIK_ERR_LEN)) {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateError
                                                        object:[NSString stringWithUTF8String:err]];
    return;
  }
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateUserLoggedOut object:nil];
}

- (IBAction)sync:(id)sender {
  NSLog(@"MainWindow sync");
  [self startSync];
}

void sync_finished(kopsik_api_result result, const char *err) {
  NSLog(@"MainWindow sync_finished");
  if (KOPSIK_API_SUCCESS != result) {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateError
                                                        object:[NSString stringWithUTF8String:err]];
    
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
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateError
                                                        object:[NSString stringWithUTF8String:err]];
    kopsik_time_entry_view_item_clear(item);
    return;
  }

  if (is_tracking) {
    TimeEntryViewItem *te = [[TimeEntryViewItem alloc] init];
    [te load:item];
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateTimerRunning object:te];
  } else {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateTimerStopped object:nil];
  }
  kopsik_time_entry_view_item_clear(item);
}

void on_model_change(kopsik_api_result result,
                     const char *errmsg,
                     KopsikModelChange *change) {
  if (KOPSIK_API_SUCCESS != result) {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateError
                                                          object:[NSString stringWithUTF8String:errmsg]];
    return;
  }

  ModelChange *mc = [[ModelChange alloc] init];
  [mc load:change];

  [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventModelChange object:mc];
}

- (void)startSync {
  NSLog(@"MainWindow startSync");
  kopsik_sync_async(ctx, 1, sync_finished);
  NSLog(@"MainWindow startSync done");
}

- (void)startWebSocket {
  NSLog(@"MainWindow startWebSocket");
  kopsik_websocket_start_async(ctx, handle_error);
  NSLog(@"MainWindow startWebSocket done");
}

@end
