//
//  IdleNotificationWindowController.h
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 06/11/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "IdleEvent.h"

@interface IdleNotificationWindowController : NSWindowController

- (void)displayIdleEvent:(IdleEvent *)idleEvent;

@property (strong, nonatomic) IdleEvent *idleEvent;

@end
