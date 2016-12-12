//
//  TimeEntryCell.m
//  Toggl Desktop on the Mac
//
//  Created by Tambet Masik on 9/26/13.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "TimeEntryCell.h"
#import "UIEvents.h"
#import "ConvertHexColor.h"

#import "toggl_api.h"

@implementation TimeEntryCell

extern void *ctx;

- (IBAction)continueTimeEntry:(id)sender
{
	NSLog(@"TimeEntryCell continueTimeEntry GUID=%@", self.GUID);

	[[NSNotificationCenter defaultCenter] postNotificationName:kCommandContinue object:self.GUID];
}

- (IBAction)toggleGroup:(id)sender
{
	if (self.Group)
	{
		[[NSNotificationCenter defaultCenter] postNotificationName:kToggleGroup object:self.GroupName];
	}
}

- (void)render:(TimeEntryViewItem *)view_item
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	self.confrimless_delete = view_item.confirmlessDelete;

	self.GUID = view_item.GUID;
	self.durationTextField.stringValue = view_item.duration;
	self.Group = view_item.Group;
	self.GroupName = view_item.GroupName;
	self.GroupOpen = view_item.GroupOpen;
	self.GroupItemCount = view_item.GroupItemCount;
	self.durationTextField.toolTip = [NSString stringWithFormat:@"%@ - %@", view_item.startTimeString, view_item.endTimeString];

	// Time entry has a description
	if (view_item.Description && [view_item.Description length] > 0)
	{
		self.descriptionTextField.stringValue = view_item.Description;
		self.descriptionTextField.toolTip = view_item.Description;
	}
	else
	{
		self.descriptionTextField.stringValue = @"(no description)";
		self.descriptionTextField.toolTip = nil;
	}

	// Set billable label
	if (YES == view_item.billable)
	{
		[self.billableFlag setHidden:NO];
	}
	else
	{
		[self.billableFlag setHidden:YES];
	}

	// Time entry tags icon
	if ([view_item.tags count])
	{
		[self.tagFlag setHidden:NO];
		self.tagFlag.toolTip = [view_item.tags componentsJoinedByString:@", "];
	}
	else
	{
		[self.tagFlag setHidden:YES];
		self.tagFlag.toolTip = nil;
	}

	// Time entry not synced icon
	[self.unsyncedIcon setHidden:!view_item.unsynced];
	self.unsyncedIcon.toolTip = view_item.Error;

	// Setup Grouped mode
	[self setupGroupMode];

	// Time entry has a project
	if (view_item.ProjectAndTaskLabel && [view_item.ProjectAndTaskLabel length] > 0)
	{
		[self.projectTextField setAttributedStringValue:[self setProjectClientLabel:view_item]];
		[self.projectTextField setHidden:NO];
		self.projectTextField.toolTip = view_item.ProjectAndTaskLabel;
		self.projectTextField.textColor =
			[ConvertHexColor hexCodeToNSColor:view_item.ProjectColor];
		return;
	}

	// Time entry has no project
	self.projectTextField.stringValue = @"";
	[self.projectTextField setHidden:YES];
	self.projectTextField.toolTip = nil;
}

- (NSMutableAttributedString *)setProjectClientLabel:(TimeEntryViewItem *)view_item
{
	NSMutableAttributedString *clientName = [[NSMutableAttributedString alloc] initWithString:view_item.ClientLabel];

	[clientName setAttributes:
	 @{
		 NSFontAttributeName : [NSFont systemFontOfSize:[NSFont systemFontSize]],
		 NSForegroundColorAttributeName:[NSColor disabledControlTextColor]
	 }
						range:NSMakeRange(0, [clientName length])];
	NSMutableAttributedString *string;
	if (view_item.TaskID != 0)
	{
		string = [[NSMutableAttributedString alloc] initWithString:[view_item.TaskLabel stringByAppendingString:@". "]];

		[string setAttributes:
		 @{
			 NSFontAttributeName : [NSFont systemFontOfSize:[NSFont systemFontSize]],
			 NSForegroundColorAttributeName:[NSColor controlTextColor]
		 }
						range:NSMakeRange(0, [string length])];

		NSMutableAttributedString *projectName = [[NSMutableAttributedString alloc] initWithString:[view_item.ProjectLabel stringByAppendingString:@" "]];

		[string appendAttributedString:projectName];
	}
	else
	{
		string = [[NSMutableAttributedString alloc] initWithString:[view_item.ProjectLabel stringByAppendingString:@" "]];
	}

	[string appendAttributedString:clientName];
	return string;
}

- (void)setupGroupMode
{
	// Default descriptionbox trail (no group icon)
	int trail = 140;
	int lead = 0;
	NSString *continueIcon = @"continue_light.pdf";
	NSString *toggleGroupIcon = @"group_icon_closed.pdf";
	NSString *toggleGroupText = [NSString stringWithFormat:@"%lld", self.GroupItemCount];

	// Default color of light gray
	NSString *fillColor = @"#FAFAFA";

	// Grouped mode background update
	if (self.GroupItemCount && self.GroupOpen && !self.Group)
	{
		// Subitems to darker gray
		fillColor = @"#f0f0f0";
		lead = 10;

		// Gray color for subitem
		NSMutableAttributedString *description = [[NSMutableAttributedString alloc] initWithString:self.descriptionTextField.stringValue];
		[description setAttributes:
		 @{
			 NSForegroundColorAttributeName:[ConvertHexColor hexCodeToNSColor:@"#696969"]
		 }
							 range:NSMakeRange(0, [description length])];

		[self.descriptionTextField setAttributedStringValue:description];
	}

	if (self.Group)
	{
		// Group icon visible
		trail = 175;
		if (self.GroupOpen)
		{
			toggleGroupIcon = @"group_icon_open.pdf";
			self.groupToggleButton.title = @"";
		}
		else
		{
			// Gray color to grouped button text
			NSMutableParagraphStyle *paragrapStyle = NSMutableParagraphStyle.new;
			paragrapStyle.alignment = kCTTextAlignmentCenter;

			NSMutableAttributedString *string = [[NSMutableAttributedString alloc] initWithString:toggleGroupText];
			[string setAttributes:
			 @{
				 NSFontAttributeName : [NSFont systemFontOfSize:9.0],
				 NSForegroundColorAttributeName:[ConvertHexColor hexCodeToNSColor:@"#a4a4a4"],
				 NSParagraphStyleAttributeName:paragrapStyle
			 }
							range:NSMakeRange(0, [string length])];

			[self.groupToggleButton setAttributedTitle:string];
		}
		[self.groupToggleButton setImage:[NSImage imageNamed:toggleGroupIcon]];

		continueIcon = @"continue_regular.pdf";
	}

	[self.continueButton setImage:[NSImage imageNamed:continueIcon]];
	[self.groupToggleButton setHidden:!self.Group];
	self.descriptionBoxLead.constant = lead;
	self.descriptionBoxTrail.constant = trail;

	[self.backgroundBox setFillColor:[ConvertHexColor hexCodeToNSColor:fillColor]];
}

- (void)focusFieldName
{
	NSPoint globalLocation = [ NSEvent mouseLocation ];
	NSPoint windowLocation = [ [ self window ] convertScreenToBase:globalLocation ];
	NSPoint mouseLocation = [ self convertPoint:windowLocation fromView:nil ];

	[self setFocused];

	if (NSPointInRect(mouseLocation, self.projectTextField.frame))
	{
		toggl_edit(ctx, [self.GUID UTF8String], false, kFocusedFieldNameProject);
		return;
	}

	if (NSPointInRect(mouseLocation, self.descriptionTextField.frame))
	{
		toggl_edit(ctx, [self.GUID UTF8String], false, kFocusedFieldNameDescription);
		return;
	}

	if (NSPointInRect(mouseLocation, self.durationBox.frame))
	{
		toggl_edit(ctx, [self.GUID UTF8String], false, kFocusedFieldNameDuration);
		return;
	}

	toggl_edit(ctx, [self.GUID UTF8String], false, "");
}

- (void)setFocused
{
	[self.backgroundBox setFillColor:[ConvertHexColor hexCodeToNSColor:@"#E8E8E8"]];
}

- (void)openEdit
{
	toggl_edit(ctx, [self.GUID UTF8String], false, "");
}

@end
