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
#import "TogglDesktop-Swift.h"

@interface TimeEntryCell ()

@property (weak) IBOutlet NSLayoutConstraint *descriptionLblLeading;
@property (weak) IBOutlet NSTextField *projectTextField;
@property (weak) IBOutlet NSImageView *billableFlag;
@property (weak) IBOutlet NSImageView *tagFlag;
@property (weak) IBOutlet NSTextField *durationTextField;
@property (weak) IBOutlet NSImageView *unsyncedIcon;
@property (weak) IBOutlet NSBox *groupBox;
@property (weak) IBOutlet NSHoverButton *continueButton;
@property (weak) IBOutlet NSBox *backgroundBox;

@end

@implementation TimeEntryCell

extern void *ctx;

- (void)awakeFromNib {
	[super awakeFromNib];

	self.continueButton.hidden = YES;

	NSRect bounds = self.view.bounds;

	// Hack
	// We don't need to remove TrackingArea and create with new size after cell's size change
	bounds.size.width = NSScreen.mainScreen.frame.size.width;
	NSTrackingArea *tracking = [[NSTrackingArea alloc]initWithRect:bounds
														   options:(NSTrackingMouseEnteredAndExited | NSTrackingActiveInKeyWindow)
															 owner:self
														  userInfo:nil];
	[self.view addTrackingArea:tracking];
}

- (void)prepareForReuse {
	[super prepareForReuse];
	self.continueButton.hidden = YES;
}

- (void)mouseEntered:(NSEvent *)event
{
	[super mouseEntered:event];
	[self updateHoverState:YES];
}

- (void)mouseExited:(NSEvent *)event
{
	[super mouseExited:event];
	[self updateHoverState:NO];
}

- (void)updateHoverState:(BOOL)isHover {
	self.continueButton.hidden = !isHover;

	if (isHover)
	{
		if (@available(macOS 10.13, *))
		{
			self.backgroundBox.fillColor = [NSColor colorNamed:@"white-background-hover-color"];
		}
		else
		{
			self.backgroundBox.fillColor = [ConvertHexColor hexCodeToNSColor:@"#f9f9f9"];
		}
	}
	else
	{
		if (@available(macOS 10.13, *))
		{
			self.backgroundBox.fillColor = [NSColor colorNamed:@"white-background-color"];
		}
		else
		{
			self.backgroundBox.fillColor = NSColor.whiteColor;
		}
	}
}

- (IBAction)continueTimeEntry:(id)sender
{
	NSLog(@"TimeEntryCell continueTimeEntry GUID=%@", self.GUID);

	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kCommandContinue object:self.GUID];
}

- (IBAction)toggleGroup:(id)sender
{
	if (self.Group)
	{
		[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kToggleGroup object:self.GroupName];
	}
}

- (void)render:(TimeEntryViewItem *)view_item
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	self.confirmless_delete = view_item.confirmlessDelete;

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
		 NSFontAttributeName : [NSFont systemFontOfSize:12],
		 NSForegroundColorAttributeName:[NSColor disabledControlTextColor]
	 }
						range:NSMakeRange(0, [clientName length])];
	NSMutableAttributedString *string;
	if (view_item.TaskID != 0)
	{
		string = [[NSMutableAttributedString alloc] initWithString:[view_item.TaskLabel stringByAppendingString:@". "]];

		[string setAttributes:
		 @{
			 NSFontAttributeName : [NSFont systemFontOfSize:12],
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

- (void)setupGroupMode {
	self.groupBox.hidden = !self.Group;
	self.descriptionLblLeading.constant = self.Group || self.GroupOpen ? 46.0 : 15.0;
}

- (void)focusFieldName
{
	NSPoint globalLocation = [NSEvent mouseLocation];
	NSRect rect = [self.view.window convertRectFromScreen:NSMakeRect(globalLocation.x, globalLocation.y, 0, 0)];
	NSPoint windowLocation = rect.origin;
	NSPoint mouseLocation = [self.view convertPoint:windowLocation fromView:nil];

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

	if (NSPointInRect(mouseLocation, self.durationTextField.frame))
	{
		toggl_edit(ctx, [self.GUID UTF8String], false, kFocusedFieldNameDuration);
		return;
	}

	toggl_edit(ctx, [self.GUID UTF8String], false, "");
}

- (void)setFocused
{
	NSColor *fillColor = self.isDarkMode ? [NSColor controlColor] : [ConvertHexColor hexCodeToNSColor:@"#E8E8E8"];

	[self.backgroundBox setFillColor:fillColor];
}

- (void)openEdit
{
	if (self.GUID != nil)
	{
		toggl_edit(ctx, [self.GUID UTF8String], false, "");
	}
}

@end
