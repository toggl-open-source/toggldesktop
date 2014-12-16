//
//  TimeEntryCellWithHeader.m
//  Toggl Desktop on the Mac
//
//  Created by Tambet Masik on 9/26/13.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "TimeEntryCellWithHeader.h"
#import "UIEvents.h"
#import "ConvertHexColor.h"

@implementation TimeEntryCellWithHeader

- (void)render:(TimeEntryViewItem *)view_item
{
	[super render:view_item];
	self.dateDurationTextField.stringValue = view_item.dateDuration;
	self.formattedDateTextField.stringValue = view_item.formattedDate;
}

@end
