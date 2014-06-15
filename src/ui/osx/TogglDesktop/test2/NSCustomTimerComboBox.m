//
//  NSCustomTimerComboBox.m
//  TogglDesktop
//
//  Created by Indrek Vändrik on 02/06/14.
//  Copyright (c) 2014 Alari. All rights reserved.
//

#import "NSCustomTimerComboBox.h"
#import "UIEvents.h"

@implementation NSCustomTimerComboBox

- (void)mouseDown:(NSEvent *)theEvent
{
	if (self.isEditable)
	{
		[[NSNotificationCenter defaultCenter] postNotificationName:kForceCloseEditPopover
															object:nil];
		return;
	}
}

@end
