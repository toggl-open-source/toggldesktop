//
//  NSTextFieldClickable.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 11/11/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "NSTextFieldClickable.h"
#import "UIEvents.h"

@implementation NSTextFieldClickable

- (void)mouseDown:(NSEvent *)theEvent
{
	if (self.isEditable)
	{
		[[NSNotificationCenter defaultCenter] postNotificationName:kForceCloseEditPopover
															object:nil];
		return;
	}
	[self sendAction:@selector(textFieldClicked:) to:[self delegate]];
}

@end
