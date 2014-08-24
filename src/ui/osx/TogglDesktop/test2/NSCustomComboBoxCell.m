//
//  NSCustomComboBoxCell.m
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 13/11/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "NSCustomComboBoxCell.h"

@implementation NSCustomComboBoxCell
- (id)init
{
	self = [super init];
	if (self)
	{
		self.cellLength = 0;
	}
	return self;
}

- (void)setCalculatedMaxWidth:(double)length
{
	self.cellLength = length;
}

- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
	if (_cellLength != 0)
	{
		NSRect myRect = NSMakeRect(cellFrame.origin.x, cellFrame.origin.y,
								   fmin(_cellLength, 500), cellFrame.size.height);

		[super drawWithFrame:myRect inView:controlView];
	}
	else
	{
		[super drawWithFrame:cellFrame inView:controlView];
	}
}

@end
