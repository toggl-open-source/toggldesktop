//
//  MainWindowController.m
//  kopsik_ui_osx
//
//  Created by Tambet Masik on 9/24/13.
//  Copyright (c) 2013 Alari. All rights reserved.
//

#import "MainWindowController.h"
#import "LoginViewController.h"
#import "TimeEntryListViewController.h"
#import "UIEvents.h"
#import "Context.h"

@interface MainWindowController ()
@property (nonatomic,strong) IBOutlet LoginViewController *loginViewController;
@property (nonatomic,strong) IBOutlet TimeEntryListViewController *timeEntryListViewController;
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
    self.loginViewController = [[LoginViewController alloc]
                                initWithNibName:@"LoginViewController" bundle:nil];
    self.timeEntryListViewController = [[TimeEntryListViewController alloc]
                                        initWithNibName:@"TimeEntryListViewController" bundle:nil];
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
  }
  kopsik_user_clear(user);
}

-(void)eventHandler: (NSNotification *) notification
{
  NSLog(@"event triggered: %@", notification.name);
  if ([notification.name isEqualToString:kUIEventUserLoggedOut]) {
    [self.timeEntryListViewController.view removeFromSuperview];
    [self.contentView addSubview:self.loginViewController.view];
    self.loginViewController.view.frame =self.contentView.bounds;
    [self.timeEntryListViewController.view removeFromSuperview];
  } else if ([notification.name isEqualToString:kUIEventUserLoggedIn]) {
    [self.loginViewController.view removeFromSuperview];
    [self.contentView addSubview:self.timeEntryListViewController.view];
    self.timeEntryListViewController.view.frame =self.contentView.bounds;
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

@end
