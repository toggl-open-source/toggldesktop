//
//  NSCustomComboBox.m
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 13/11/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "NSCustomComboBox.h"
#import "NSCustomComboBoxCell.h"

@implementation NSCustomComboBox

+ (void)load
{
	[self setCellClass:[NSCustomComboBoxCell class]];
}

- (void)reloadingData:(NSNumber *)length
{
	[super reloadData];
	int n = [length intValue];
	[self.cell setCalculatedMaxWidth:fmax(8 * n, self.frame.size.width)];
}

- (BOOL)becomeFirstResponder
{
	BOOL success = [super becomeFirstResponder];

	if (success)
	{
		NSTextView *textField = (NSTextView *)[self currentEditor];
		if ([textField respondsToSelector:@selector(setInsertionPointColor:)])
		{
			[textField setInsertionPointColor:[NSColor blackColor]];
		}
	}
	return success;
}

- (BOOL)isExpanded
{
	id ax = NSAccessibilityUnignoredDescendant(self);

	return [[ax accessibilityAttributeValue:
			 NSAccessibilityExpandedAttribute] boolValue];
}

@end
