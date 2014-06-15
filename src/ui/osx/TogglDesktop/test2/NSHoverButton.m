//
//  NSHoverButton.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 21/02/2014.
//  Copyright (c) 2014 Toggl Desktop developers. All rights reserved.
//

#import "NSHoverButton.h"
#import "ConvertHexColor.h"

@implementation NSHoverButton

// http://stackoverflow.com/questions/7889419/cocoa-button-rollovers-with-mouseentered-and-mouseexited
- (void)awakeFromNib
{
	NSTrackingAreaOptions focusTrackingAreaOptions = NSTrackingActiveInActiveApp;

	focusTrackingAreaOptions |= NSTrackingMouseEnteredAndExited;
	focusTrackingAreaOptions |= NSTrackingAssumeInside;
	focusTrackingAreaOptions |= NSTrackingInVisibleRect;

	NSTrackingArea *focusTrackingArea = [[NSTrackingArea alloc] initWithRect:NSZeroRect
																	 options:focusTrackingAreaOptions owner:self userInfo:nil];
	[self addTrackingArea:focusTrackingArea];
}

- (void)mouseEntered:(NSEvent *)theEvent
{
	[self.animator setAlphaValue:0.5];
}

- (void)mouseExited:(NSEvent *)theEvent
{
	[self.animator setAlphaValue:1];
}

@end
