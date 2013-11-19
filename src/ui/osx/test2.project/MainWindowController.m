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
#import "Update.h"
#import "MenuItemTags.h"

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
  }
  return self;
}

- (void)windowDidLoad
{
  [super windowDidLoad];
}

-(void)eventHandler: (NSNotification *) notification
{
  if ([notification.name isEqualToString:kUIStateUserLoggedIn]) {
    User *userinfo = notification.object;
    [Bugsnag setUserAttribute:@"user_id" withValue:[NSString stringWithFormat:@"%ld", userinfo.ID]];
    
    // Hide login view
    [self.loginViewController.view removeFromSuperview];

    // Show time entry list
    [self.contentView addSubview:self.timeEntryListViewController.view];
    [self.timeEntryListViewController.view setFrame:self.contentView.bounds];
    
    // Show header
    [self.headerView setHidden:NO];
    return;
  }
  
  if ([notification.name isEqualToString:kUIStateUserLoggedOut]) {
    [Bugsnag setUserAttribute:@"user_id" withValue:nil];

    // Show login view
    [self.contentView addSubview:self.loginViewController.view];
    [self.loginViewController.view setFrame:self.contentView.bounds];
    [self.loginViewController.email becomeFirstResponder];

    // Hide all other views
    [self.timeEntryListViewController.view removeFromSuperview];
    [self.headerView setHidden:YES];
    [self.timerViewController.view removeFromSuperview];
    return;
  }
  
  if ([notification.name isEqualToString:kUIStateTimerRunning]) {
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
    return;
  }
  
  if ([notification.name isEqualToString:kUIStateTimerStopped]) {
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
    return;
  }

  if ([notification.name isEqualToString:kUIStateTimeEntrySelected]) {
    [self.headerView setHidden:YES];
    [self.timeEntryListViewController.view removeFromSuperview];
    [self.contentView addSubview:self.timeEntryEditViewController.view];
    [self.timeEntryEditViewController.view setFrame:self.contentView.bounds];
    return;
  }

  if ([notification.name isEqualToString:kUIStateTimeEntryDeselected]) {
    [self.headerView setHidden:NO];
    [self.timeEntryEditViewController.view removeFromSuperview];
    [self.contentView addSubview:self.timeEntryListViewController.view];
    [self.timeEntryListViewController.view setFrame:self.contentView.bounds];
    return;
  }
 
  if ([notification.name isEqualToString:kUIStateError]) {
    // Proxy all app errors through this notification.

    NSString *msg = notification.object;
    NSLog(@"Error: %@", msg);

    // FIXME: move these error messages into lib, so we can reuse them in other platforms.

    // Ignore offline errors
    if ([msg rangeOfString:@"Host not found"].location != NSNotFound) {
      return;
    }
    if ([msg rangeOfString:@"WebSocket Exception: Cannot upgrade to WebSocket connection: OK"].location != NSNotFound) {
      return;
    }
    if ([msg rangeOfString:@"No message received"].location != NSNotFound) {
      return;
    }
    if ([msg rangeOfString:@"Connection refused"].location != NSNotFound) {
      return;
    }
    if ([msg rangeOfString:@"Connection timed out"].location != NSNotFound) {
      return;
    }
    
    [self performSelectorOnMainThread:@selector(showError:) withObject:msg waitUntilDone:NO];

    [Bugsnag notify:[NSException
                     exceptionWithName:@"UI error"
                     reason:msg
                     userInfo:nil]];
    return;
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

- (void)windowWillClose:(NSNotification *)notification {
  ProcessSerialNumber psn = { 0, kCurrentProcess };
  TransformProcessType(&psn, kProcessTransformToUIElementApplication);
}

@end
