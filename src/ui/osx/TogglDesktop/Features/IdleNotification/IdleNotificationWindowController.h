//
//  IdleNotificationWindowController.h
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 06/11/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class IdleEvent;

@interface IdleNotificationWindowController : NSWindowController
@property (nonatomic, strong) IdleEvent *idleEvent;
- (void)displayIdleEvent:(IdleEvent *)idleEvent;
@end
