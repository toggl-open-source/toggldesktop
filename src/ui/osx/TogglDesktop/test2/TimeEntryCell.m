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
#import <QuartzCore/CAShapeLayer.h>

@interface TimeEntryCell ()

@property (weak) IBOutlet NSLayoutConstraint *descriptionLblLeading;
@property (weak) IBOutlet NSTextField *projectTextField;
@property (weak) IBOutlet NSImageView *billableFlag;
@property (weak) IBOutlet NSImageView *tagFlag;
@property (weak) IBOutlet NSTextField *durationTextField;
@property (weak) IBOutlet NSImageView *unsyncedIcon;
@property (weak) IBOutlet NSBox *groupBox;
@property (weak) IBOutlet NSButton *groupButton;
@property (weak) IBOutlet NSButton *continueButton;
@property (weak) IBOutlet NSBox *backgroundBox;
@property (weak) IBOutlet DotImageView *dotView;
@property (weak) IBOutlet NSLayoutConstraint *projectConstrainLeading;
@property (weak) IBOutlet NSBox *horizontalLine;

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
	[self resetMask];
	[self updateHoverState:NO];
}

- (void)mouseEntered:(NSEvent *)event
{
	[super mouseEntered:event];

	// Only pply hover color if it's not sub-items
	if (self.cellType != CellTypeSubItemInGroup)
	{
		[self updateHoverState:YES];
	}

	// Continue
	self.continueButton.hidden = NO;
}

- (void)mouseExited:(NSEvent *)event
{
	[super mouseExited:event];

	// Only pply hover color if it's not sub-items
	if (self.cellType != CellTypeSubItemInGroup)
	{
		[self updateHoverState:NO];
	}

	// Continue
	self.continueButton.hidden = YES;
}

- (void)updateHoverState:(BOOL)isHover {
	if (isHover)
	{
		self.backgroundBox.transparent = NO;
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
		self.backgroundBox.transparent = YES;
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

	// Cell type
	if (self.Group)
	{
		self.cellType = CellTypeGroup;
	}
	else if (!self.Group && self.GroupOpen)
	{
		self.cellType = CellTypeSubItemInGroup;
	}
	else
	{
		self.cellType = CellTypeNormal;
	}

	// Time entry has a description
	if (view_item.Description && [view_item.Description length] > 0)
	{
		self.descriptionTextField.stringValue = view_item.Description;
		self.descriptionTextField.toolTip = view_item.Description;
	}
	else
	{
		self.descriptionTextField.stringValue = @"";
		self.descriptionTextField.placeholderString = @"+ Add description";
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
		NSColor *projectColor = [ConvertHexColor hexCodeToNSColor:view_item.ProjectColor];
		self.dotView.hidden = NO;
		[self.dotView fillWith:projectColor];
		[self.projectTextField setAttributedStringValue:[self setProjectClientLabel:view_item]];
		self.projectTextField.toolTip = view_item.ProjectAndTaskLabel;
		self.projectTextField.textColor = projectColor;
		self.projectConstrainLeading.constant = 16;
		return;
	}

	// Time entry has no project
	self.projectTextField.stringValue = @"";
	self.projectTextField.placeholderString = @"+ Add project";
	self.projectTextField.toolTip = nil;
	self.dotView.hidden = YES;
	self.projectConstrainLeading.constant = 0;
}

- (NSMutableAttributedString *)setProjectClientLabel:(TimeEntryViewItem *)view_item
{
	NSString *clientTitle = [NSString stringWithFormat:@"• %@", view_item.ClientLabel];
	NSMutableAttributedString *clientName = [[NSMutableAttributedString alloc] initWithString:clientTitle];

	[clientName setAttributes:
	 @{
		 NSFontAttributeName : [NSFont systemFontOfSize:12],
		 NSForegroundColorAttributeName:[self clientTextColor]
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

	// Title
	NSString *toggleGroupText = [NSString stringWithFormat:@"%lld", self.GroupItemCount];
	self.groupButton.title = toggleGroupText;

	// Color
	if (self.Group && self.GroupOpen)
	{
		if (@available(macOS 10.13, *))
		{
			[self.groupButton setTextColor:[NSColor colorNamed:@"green-color"]];
			self.groupBox.fillColor = [NSColor colorNamed:@"group-box-background-color"];
		}
		else
		{
			[self.groupButton setTextColor:[ConvertHexColor hexCodeToNSColor:@"#28cd41"]];
			self.groupBox.fillColor = [NSColor colorWithRed:40.0 / 255.0 green:205.0 / 255.0 blue:65.0 / 255.0 alpha:0.12];
		}
		self.groupBox.borderColor = [NSColor clearColor];
	}
	else
	{
		if (@available(macOS 10.13, *))
		{
			[self.groupButton setTextColor:[NSColor colorNamed:@"grey-text-color"]];
			self.groupBox.fillColor = [NSColor colorNamed:@"upload-background-color"];
			self.groupBox.borderColor = [NSColor colorNamed:@"upload-border-color"];
		}
		else
		{
			[self.groupButton setTextColor:[ConvertHexColor hexCodeToNSColor:@"#555555"]];
			self.groupBox.fillColor = [NSColor whiteColor];
			self.groupBox.borderColor = [NSColor lightGrayColor];
		}
	}

	if (self.cellType == CellTypeSubItemInGroup)
	{
		[self updateHoverState:YES];
	}
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
	[self updateHoverState:YES];
}

- (void)openEdit
{
	if (self.GUID != nil)
	{
		toggl_edit(ctx, [self.GUID UTF8String], false, "");
	}
}

- (NSColor *)clientTextColor
{
	if (@available(macOS 10.13, *))
	{
		return [NSColor colorNamed:@"grey-text-color"];
	}
	else
	{
		return [ConvertHexColor hexCodeToNSColor:@"#555555"];
	}
}

- (void)showHorizontalLine:(BOOL)show
{
	self.horizontalLine.hidden = !show;
}

- (void)applyMaskForBottomCorner {
	self.backgroundBox.wantsLayer = YES;
	self.backgroundBox.layer.mask = [self maskFor:PositionBottom rect:self.view.bounds cornerRadius:4.0];
}

- (void)resetMask {
	self.backgroundBox.layer.mask = nil;
	self.backgroundBox.wantsLayer = NO;
}

// - (void)mouseDown:(NSEvent *)event {
//    [super mouseDown:event];
// }
//
// - (IBAction)performClick:(id)sender {
//    NSLog(@"performClick");
////    if timeEntryCell.cellType == .group {
////        NotificationCenter.default.postNotificationOnMainThread(NSNotification.Name(kToggleGroup),
////                                                                object: timeEntryCell.groupName)
////        return
////    }
////    timeEntryCell.focusFieldName()
//
//    if (self.cellType == CellTypeGroup)
//    {
//        [[NSNotificationCenter defaultCenter] postNotificationName:kToggleGroup object:self.GroupName];
//    }
//    else
//    {
//        [self focusFieldName];
//    }
//
//
// }

@end
