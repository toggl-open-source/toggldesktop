//
//  MKColorWell+Custom.m
//  TogglDesktop
//
//  Created by Indrek Vändrik on 17/12/15.
//  Copyright (c) 2015 Toggl Desktop developers. All rights reserved.
//

#import "MKColorWellCustom.h"
#import "ConvertHexColor.h"
#import "MKColorPickerView.h"
#import "Utils.h"

@implementation MKColorWellCustom

- (void)setProjectColors:(NSMutableArray *)colorStrings;
{
	self.hexColors = colorStrings;
	NSMutableArray *colorCodes = [NSMutableArray array];
	for (NSString *c in colorStrings)
	{
		[colorCodes addObject:[ConvertHexColor hexCodeToNSColor:c]];
	}

	self.colors = colorCodes;
	self.color = self.colors[0];
	[self setupPopover];
}

- (NSString *)getSelectedColor
{
	NSInteger i = 0;

	for (NSColor *c in self.colors)
	{
		if ([self.color isEqualTo:c])
		{
			return self.hexColors[i];
		}
		i++;
	}
	return self.hexColors[0];
}

- (NSArray *)colorsForPopover
{
	return self.colors;
}

- (NSPopover *)createPopover
{
	NSPopover *aPopover = [[NSPopover alloc] init];

	[aPopover setBehavior:NSPopoverBehaviorSemitransient];
	[aPopover setAnimates:self.animatePopover];
	return aPopover;
}

- (NSViewController *)createPopoverViewController
{
	NSViewController *aPopoverViewController = [[NSViewController alloc] init];

	return aPopoverViewController;
}

- (MKColorPickerView *)createPopoverView
{
	NSArray *colors = [self colorsForPopover];
	uint rows = 5;
	NSSize swatchSize = NSMakeSize(20, 20);

	MKColorPickerView *aPopoverView;

	aPopoverView = [[MKColorPickerView alloc] initWithColors:colors
												numberOfRows:rows
											 numberOfColumns:[colors count] / rows
												  swatchSize:swatchSize
											 targetColorWell:self];
	return aPopoverView;
}

- (void)setupPopover
{
	if (self.colors != nil)
	{
		popoverViewController = [self createPopoverViewController];
		popoverView = [self createPopoverView];

		popover.contentViewController = popoverViewController;
		popoverViewController.view = popoverView;
	}
	else
	{
		popover = [self createPopover];
	}
}

- (void)drawRect:(NSRect)dirtyRect
{
	[[NSColor clearColor] set];
	NSRectFillUsingOperation([self bounds], NSCompositeSourceOver);

	// Set the color in the current graphics context for future draw operations
	[self.color setStroke];
	[self.color setFill];

	// Create our circle path
	NSRect rect = NSMakeRect(6, 6, 12, 12);
	NSBezierPath *circlePath = [NSBezierPath bezierPath];
	[circlePath appendBezierPathWithOvalInRect:rect];

	// Outline and fill the path
	[circlePath stroke];
	[circlePath fill];
}

@end
