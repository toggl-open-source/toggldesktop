//
//  NSTextFieldDuration.m
//  TogglDesktop
//
//  Created by Indrek Vändrik on 05/12/14.
//  Copyright (c) 2014 Alari. All rights reserved.
//

#import "NSTextFieldDuration.h"
#import "UIEvents.h"

@implementation NSTextFieldDuration

- (void)mouseDown:(NSEvent *)theEvent
{
	[self sendAction:@selector(textFieldClicked:) to:[self delegate]];
}

- (BOOL)becomeFirstResponder
{
	BOOL success = [super becomeFirstResponder];

	if (success && self.isEditable)
	{
		NSTextView *textField = (NSTextView *)[self currentEditor];
		if ([textField respondsToSelector:@selector(setInsertionPointColor:)])
		{
			[textField setInsertionPointColor:[NSColor whiteColor]];
		}
	}
	return success;
}

- (BOOL)isExpanded
{
	return NO;
}

@end
