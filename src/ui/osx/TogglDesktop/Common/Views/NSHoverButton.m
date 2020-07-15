//
//  NSHoverButton.m
//  Toggl Desktop on the Mac
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
	self.alpha = 0.5;
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
	[self.animator setAlphaValue:self.alpha];
}

- (void)mouseExited:(NSEvent *)theEvent
{
	[self.animator setAlphaValue:1];
}

- (void)setHoverAlpha:(CGFloat)alphaValue
{
	self.alpha = alphaValue;
}

@end
