//
//  AppDelegate.m
//  test2
//
//  Created by Alari on 9/15/13.
//  Copyright (c) 2013 Alari. All rights reserved.
//

#import "AppDelegate.h"
#include "kopsik_api.h"

#include "MainWindowController.h"

@interface  AppDelegate()
@property (nonatomic,strong) IBOutlet MainWindowController *mainWindowController;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
  self.mainWindowController = [[MainWindowController alloc] initWithWindowNibName:@"MainWindowController"];
  self.mainWindowController->ctx_ = ctx_;
  [self.mainWindowController showWindow:self];
  
  int major = 0;
  int minor = 0;
  int patch = 0;
  kopsik_version(&major, &minor, &patch);
  kopsik_set_app_path(ctx_, [self.applicationSupportDirectory UTF8String]);
  NSString *s = [NSString stringWithFormat:@"libkopsik version %d.%d.%d", major, minor, patch];
  NSLog(@"%@", s);
}

- (NSString *)applicationSupportDirectory
{
  NSString *path;
  NSError *error;
  NSArray* paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
  if ([paths count] == 0) {
    NSLog(@"Unable to access application support directory!");
  }
  path = [[paths objectAtIndex:0] stringByAppendingPathComponent:@"libkopsik"];
  
	if (![[NSFileManager defaultManager] fileExistsAtPath:path]){
		if (![[NSFileManager defaultManager] createDirectoryAtPath:path
                                   withIntermediateDirectories:NO
                                                    attributes:nil
                                                         error:&error]){
			NSLog(@"Create directory error: %@", error);
		}
	}
  return path;
}

- (id) init
{
  self = [super init];
  ctx_ = kopsik_context_init();
  return self;
}

- (void) dealloc
{
  kopsik_context_clear(ctx_);
}

@end
