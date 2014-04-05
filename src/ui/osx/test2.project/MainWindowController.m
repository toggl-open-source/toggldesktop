//
//  MainWindowController.m
//  kopsik_ui_osx
//
//  Created by Tambet Masik on 9/24/13.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "MainWindowController.h"
#import "LoginViewController.h"
#import "TimeEntryListViewController.h"
#import "TimeEntryEditViewController.h"
#import "TimeEntryViewItem.h"
#import "UIEvents.h"
#import "Core.h"
#import "Bugsnag.h"
#import "User.h"
#import "ModelChange.h"
#import "ErrorHandler.h"
#import "Update.h"
#import "MenuItemTags.h"

@interface MainWindowController ()
@property (nonatomic,strong) IBOutlet LoginViewController *loginViewController;
@property (nonatomic,strong) IBOutlet TimeEntryListViewController *timeEntryListViewController;
@property (nonatomic,strong) IBOutlet TimeEntryEditViewController *timeEntryEditViewController;
@end

@implementation MainWindowController

- (id)initWithWindow:(NSWindow *)window {
  self = [super initWithWindow:window];
  if (self) {
    self.loginViewController = [[LoginViewController alloc]
                                initWithNibName:@"LoginViewController" bundle:nil];
    self.timeEntryListViewController = [[TimeEntryListViewController alloc]
                                        initWithNibName:@"TimeEntryListViewController" bundle:nil];
    self.timeEntryEditViewController = [[TimeEntryEditViewController alloc]
                                    initWithNibName:@"TimeEntryEditViewController" bundle:nil];
    
    [self.loginViewController.view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
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
                                                 name:kUIEventSettingsChanged
                                               object:nil];
  }
  return self;
}

- (void)eventHandler: (NSNotification *) notification {
  if ([notification.name isEqualToString:kUIStateUserLoggedIn]) {
    User *userinfo = notification.object;
    [Bugsnag configuration].userId = [NSString stringWithFormat:@"%ld", userinfo.ID];
    
    // Hide login view
    [self.loginViewController.view removeFromSuperview];

    // Show time entry list
    [self.contentView addSubview:self.timeEntryListViewController.view];
    [self.timeEntryListViewController.view setFrame:self.contentView.bounds];
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventShowListView object:nil];
    
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
    [self.timeEntryEditViewController.view removeFromSuperview];
    return;
  }
    
  if ([notification.name isEqualToString:kUIStateTimeEntrySelected]) {
    [self.timeEntryListViewController.view removeFromSuperview];
    [self.contentView addSubview:self.timeEntryEditViewController.view];
    [self.timeEntryEditViewController.view setFrame:self.contentView.bounds];
    return;
  }

  if ([notification.name isEqualToString:kUIStateTimeEntryDeselected]) {
    [self.timeEntryEditViewController.view removeFromSuperview];
    [self.contentView addSubview:self.timeEntryListViewController.view];
    [self.timeEntryListViewController.view setFrame:self.contentView.bounds];
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventShowListView object:nil];
    return;
  }
 
  if ([notification.name isEqualToString:kUIStateError]) {
    // Proxy all app errors through this notification.

    NSString *msg = notification.object;
    NSLog(@"Error: %@", msg);

    if (kopsik_is_networking_error([msg UTF8String])) {
      [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateOffline object:nil];
      return;
    }
    
    if ([msg rangeOfString:@"Request to server failed with status code: 403"].location != NSNotFound) {
      _Bool logged_in = false;
      if (kopsik_user_is_logged_in(ctx, &logged_in)) {
        if (!logged_in) {
          return; // because login screen handles it
        }
        msg = @"It seems your login credentials have changed in Toggl.com. Please try to logout/login again from TogglDesktop.";
      }
    }
    
    [self performSelectorOnMainThread:@selector(showError:) withObject:msg waitUntilDone:NO];

    [Bugsnag notify:[NSException
                     exceptionWithName:msg
                     reason:msg
                     userInfo:nil]];
    return;
  }
}

-(void)windowDidLoad {
    // Make the window visible on all Spaces
    // http://stackoverflow.com/questions/7458353/cocoa-programmatically-adding-an-application-to-all-spaces
    if ([[self window] respondsToSelector: @selector(setCollectionBehavior:)]) {
        [[self window] setCollectionBehavior: NSWindowCollectionBehaviorCanJoinAllSpaces];
    } else if ([[self window] respondsToSelector: @selector(canBeVisibleOnAllSpaces)]) {
        [[self window] canBeVisibleOnAllSpaces]; // AVAILABLE_MAC_OS_X_VERSION_10_5_AND_LATER_BUT_DEPRECATED
    }
}

- (void)showError:(NSString *)msg {
  NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

  NSAlert *alert = [[NSAlert alloc] init];
  [alert setMessageText:msg];
  [alert addButtonWithTitle:@"Dismiss"];
  [alert beginSheetModalForWindow:self.window
                    modalDelegate:self
                   didEndSelector:nil
                      contextInfo:nil];
}

@end
