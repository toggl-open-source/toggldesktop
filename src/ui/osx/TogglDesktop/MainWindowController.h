//
//  MainWindowController.h
//  Toggl Desktop on the Mac
//
//  Created by Tambet Masik on 9/24/13.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "toggl_api.h"

/**
 * The level of window.
 *
 * - WindowModeAlwaysOnTop: Always in top (NSFloatingWindowLevel)
 * - WindowModeDefault: Normal behavior (NSNormalWindowLevel)
 */
typedef NS_ENUM (NSUInteger, WindowMode)
{
	WindowModeAlwaysOnTop,
	WindowModeDefault,
};

@interface MainWindowController : NSWindowController

- (BOOL)isEditOpened;
- (void)setWindowMode:(WindowMode)mode;

@end
