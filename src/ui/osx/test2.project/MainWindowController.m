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
#import "Update.h"
#import "MenuItemTags.h"
#import "DisplayCommand.h"

@interface MainWindowController ()
@property (nonatomic,strong) IBOutlet LoginViewController *loginViewController;
@property (nonatomic,strong) IBOutlet TimeEntryListViewController *timeEntryListViewController;
@property (nonatomic,strong) IBOutlet TimeEntryEditViewController *timeEntryEditViewController;
@end

@implementation MainWindowController

extern void *ctx;

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
                                             selector:@selector(startDisplayLogin:)
                                                 name:kDisplayLogin
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(startDisplayTimeEntryEditor:)
                                                 name:kDisplayTimeEntryEditor
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(startDisplayTimeEntryList:)
                                                 name:kDisplayTimeEntryList
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(startDisplayError:)
                                                 name:kDisplayError
                                               object:nil];
  }
  return self;
}

- (void)startDisplayLogin:(NSNotification *)notification {
  [self performSelectorOnMainThread:@selector(displayLogin:)
                         withObject:notification.object
                      waitUntilDone:NO];
}

- (void)displayLogin:(DisplayCommand *)cmd {
  NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");
  if (cmd.open) {
    [self.contentView addSubview:self.loginViewController.view];
    [self.loginViewController.view setFrame:self.contentView.bounds];
    [self.loginViewController.email becomeFirstResponder];
    
    [self.timeEntryListViewController.view removeFromSuperview];
    [self.timeEntryEditViewController.view removeFromSuperview];
  }
}

- (void)addUnderlineToTextField:(NSTextField *)field {
  NSMutableAttributedString *forgot = [[field attributedStringValue] mutableCopy];
  [forgot addAttribute:NSUnderlineStyleAttributeName
      value:[NSNumber numberWithInt:NSUnderlineStyleSingle]
      range:NSMakeRange(0, forgot.length)];
  [field setAttributedStringValue:forgot];
}

- (void)startDisplayTimeEntryEditor:(NSNotification *)notification {
  [self performSelectorOnMainThread:@selector(displayTimeEntryEditor:)
                         withObject:notification.object
                      waitUntilDone:NO];
}

- (void)displayTimeEntryEditor:(DisplayCommand *)cmd {
  NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");
  if (cmd.open) {
    [self.contentView addSubview:self.timeEntryEditViewController.view];
    [self.timeEntryEditViewController.view setFrame:self.contentView.bounds];

    [self.loginViewController.view removeFromSuperview];
    [self.timeEntryListViewController.view removeFromSuperview];
  }
}

- (void)startDisplayTimeEntryList:(NSNotification *)notification {
  [self performSelectorOnMainThread:@selector(displayTimeEntryList:)
                         withObject:notification.object
                      waitUntilDone:NO];
}

- (void)displayTimeEntryList:(DisplayCommand *)cmd {
  NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");
  if (cmd.open) {
    [self.contentView addSubview:self.timeEntryListViewController.view];
    [self.timeEntryListViewController.view setFrame:self.contentView.bounds];

    [self.loginViewController.view removeFromSuperview];
    [self.timeEntryEditViewController.view removeFromSuperview];
  }
}

- (void)startDisplayError:(NSNotification *)notification {
  [self performSelectorOnMainThread:@selector(displayError:)
                         withObject:notification.object
                      waitUntilDone:NO];
}

- (void)displayError:(NSString *)msg {
  NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

  if ([msg rangeOfString:@"Invalid e-mail or password"].location != NSNotFound) {
    [self.errorLink setStringValue:@"Forgot your password?"];
    [self addUnderlineToTextField:self.errorLink];
  }

  [self.errorLabel setStringValue:msg];
  [self.troubleBox setHidden:NO];
}

- (void)closeError {
  NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");
  [self.troubleBox setHidden:YES];
  [self.errorLabel setStringValue:@""];
  [self.errorLink setStringValue:@""];
}

- (IBAction)errorCloseButtonClicked:(id)sender {
    [self closeError];
}

-(void)textFieldClicked:(id)sender {
  if (sender == self.errorLink) {
    kopsik_password_forgot(ctx);
  }
}

@end
