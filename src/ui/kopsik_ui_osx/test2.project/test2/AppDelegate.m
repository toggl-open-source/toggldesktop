//
//  AppDelegate.m
//  test2
//
//  Created by Alari on 9/15/13.
//  Copyright (c) 2013 Alari. All rights reserved.
//

#import "AppDelegate.h"
#import "kopsik_api.h"
#import "Context.h"
#import "MainWindowController.h"

@interface  AppDelegate()
@property (nonatomic,strong) IBOutlet MainWindowController *mainWindowController;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
  self.mainWindowController = [[MainWindowController alloc] initWithWindowNibName:@"MainWindowController"];
  [self.mainWindowController showWindow:self];
  
  int major = 0;
  int minor = 0;
  int patch = 0;
  kopsik_version(&major, &minor, &patch);
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
  path = [[paths objectAtIndex:0] stringByAppendingPathComponent:@"Kopsik"];
  
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
  ctx = kopsik_context_init();

  NSString *app_path = self.applicationSupportDirectory;

  NSString *db_path = [app_path stringByAppendingPathComponent:@"kopsik.db"];
  kopsik_set_db_path(ctx, [db_path UTF8String]);

  NSString *log_path = [app_path stringByAppendingPathComponent:@"kopsik.log"];
  kopsik_set_log_path(ctx, [log_path UTF8String]);

  return self;
}

- (void) dealloc
{
  kopsik_context_clear(ctx);
  ctx = 0;
}

@end
