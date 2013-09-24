//
//  AppDelegate.m
//  test2
//
//  Created by Alari on 9/15/13.
//  Copyright (c) 2013 Alari. All rights reserved.
//

#import "AppDelegate.h"
#include "kopsik_api.h"

#include "MasterViewController.h"

@interface  AppDelegate()
@property (nonatomic,strong) IBOutlet MasterViewController *masterViewController;
@end

#define ERRLEN 1024

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
  self.masterViewController = [[MasterViewController alloc] initWithNibName:@"MasterViewController" bundle:nil];
  
  [self.window.contentView addSubview:self.masterViewController.view];
  self.masterViewController.view.frame = ((NSView*)self.window.contentView).bounds;
  
  int major = 0;
  int minor = 0;
  int patch = 0;
  kopsik_version(&major, &minor, &patch);
  NSString *s = [NSString stringWithFormat:@"libkopsik version %d.%d.%d", major, minor, patch];
  NSLog(@"%@", s);
  
  char err[ERRLEN];
  TogglUser *user = kopsik_user_new();
  if (KOPSIK_API_SUCCESS != kopsik_current_user(err, ERRLEN, user)) {
    NSLog(@"Error fetching user: %s", err);
  } else if (!user->ID) {
    NSLog(@"User not found.. LOG IN!!!");
  } else {
    NSLog(@"Current user: %s", user->Fullname);
  }
  kopsik_user_delete(user);
}

@end
