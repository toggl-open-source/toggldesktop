//
//  NSHoverButton.m
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 21/02/2014.
//  Copyright (c) 2014 Toggl Desktop developers. All rights reserved.
//

#import "NSHoverButton.h"
#import "ConvertHexColor.h"

@interface NSHoverButton ()
@property (nonatomic, strong) NSImage *originalImage;
@end

@implementation NSHoverButton

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
    self.originalImage = self.image;
    [self.animator setImage:self.hoverImage];
}

- (void)mouseExited:(NSEvent *)theEvent
{
    if (self.originalImage) {
        [self.animator setImage:self.originalImage];
    }
}

- (void)mouseDown:(NSEvent *)event {
    [self setState:NSControlStateValueOn];
    // not calling `super` so we can receive `mouseUp` event
}

- (void)mouseUp:(NSEvent *)event {
    [super mouseUp:event];

    if ([self isEnabled] != YES)
    {
        return;
    }

    NSPoint mousePoint = [self convertPoint:event.locationInWindow fromView:nil];
    BOOL isUpInside = CGRectContainsPoint(self.bounds, mousePoint);
    if (isUpInside)
    {
        [self sendAction:self.action to:self.target];
    }
    [self setState:NSControlStateValueOff];
}

- (void)setState:(NSControlStateValue)state {
    [super setState:state];
    if (state == NSControlStateValueOff) {
        self.originalImage = nil;
    }
}

- (BOOL)canBecomeKeyView {
    return YES;
}

@end
