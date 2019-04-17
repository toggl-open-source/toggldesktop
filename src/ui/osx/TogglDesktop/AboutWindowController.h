//
//  AboutWindowController.h
//  Toggl Desktop on the Mac
//
//  Copyright (c) 2014 TogglDesktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "DisplayCommand.h"

@interface AboutWindowController : NSWindowController
@property (assign, nonatomic) BOOL windowHasLoad;
@property (assign, nonatomic) BOOL restart;
@property (copy, nonatomic) NSString *updateStatus;

- (BOOL)isVisible;
@end
