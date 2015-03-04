//
//  NSTextFieldRoundedCorners.m
//  TogglDesktop
//
//  Created by Indrek Vändrik on 04/03/15.
//  Copyright (c) 2015 TogglDesktop developers. All rights reserved.
//

#import "NSTextFieldRoundedCorners.h"

@implementation NSTextFieldRoundedCorners

- (void)awakeFromNib
{
	[[self cell] setBezelStyle:NSTextFieldRoundedBezel];
}

- (void)drawRect:(NSRect)dirtyRect
{
	NSRect outlineFrame = NSMakeRect(0.0, 0.0, [self bounds].size.width, [self bounds].size.height - 1.0);
	NSGradient *gradient = [[NSGradient alloc] initWithStartingColor:self.backgroundColor endingColor:self.backgroundColor];

	[gradient drawInBezierPath:[NSBezierPath bezierPathWithRoundedRect:outlineFrame xRadius:2 yRadius:2] angle:190];
}

@end
