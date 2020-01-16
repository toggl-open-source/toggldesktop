//
//  DesktopLibraryBridge.m
//  TogglDesktop
//
//  Created by Nghia Tran on 4/2/19.
//  Copyright © 2019 Alari. All rights reserved.
//

#import "DesktopLibraryBridge.h"
#import "TimeEntryViewItem.h"
#import "toggl_api.h"
#import "AutocompleteItem.h"
#import "Utils.h"
#import "UIEvents.h"
#import "TogglDesktop-Swift.h"

@implementation DesktopLibraryBridge

void *ctx;

+ (instancetype)shared
{
	static DesktopLibraryBridge *instance = nil;
	static dispatch_once_t onceToken;

	dispatch_once(&onceToken, ^{
		instance = [[DesktopLibraryBridge alloc] init];
	});
	return instance;
}

- (NSString *)createClientWithWorkspaceID:(uint64_t)workspaceID
							   clientName:(NSString *)clientName
{
	char *clientGUID = toggl_create_client(ctx,
										   workspaceID,
										   [clientName UTF8String]);
	NSString *guid = [NSString stringWithUTF8String:clientGUID];

	free(clientGUID);
	return guid;
}

- (NSString *)createProjectWithTimeEntryGUID:(NSString *)timeEntryGUID
								 workspaceID:(uint64_t)workspaceID
									clientID:(uint64_t)clientID
								  clientGUID:(NSString *)clientGUID
								 projectName:(NSString *)projectName
									colorHex:(NSString *)colorHex
									isPublic:(BOOL)isPublic
{
	char_t *projectGUID = toggl_add_project(ctx,
											[timeEntryGUID UTF8String],
											workspaceID,
											clientID,
											[clientGUID UTF8String],
											[projectName UTF8String],
											!isPublic,
											[colorHex UTF8String]);
	NSString *guid = [NSString stringWithUTF8String:projectGUID];

	free(projectGUID);
	return guid;
}

- (void)setBillableForTimeEntryWithTimeEntryGUID:(NSString *)timeEntryGUID
									  isBillable:(BOOL)isBillable
{
	toggl_set_time_entry_billable(ctx, [timeEntryGUID UTF8String], isBillable);
}

- (void)setProjectForTimeEntryWithGUID:(NSString *)guid
								taskID:(uint64_t)taskID
							 projectID:(uint64_t)projectID
						   projectGUID:(NSString *)projectGUID
{
	toggl_set_time_entry_project(ctx,
								 [guid UTF8String],
								 taskID,
								 projectID,
								 [projectGUID UTF8String]);
}

- (void)togglEditor
{
	toggl_view_time_entry_list(ctx);
}

- (void)updateTimeEntryWithDescription:(NSString *)descriptionName guid:(NSString *)guid
{
	toggl_set_time_entry_description(ctx,
									 [guid UTF8String],
									 [descriptionName UTF8String]);
}

- (void)updateTimeEntryWithTags:(NSArray<NSString *> *)tags guid:(NSString *)guid
{
	const char *value = [[tags componentsJoinedByString:@"\t"] UTF8String];

	toggl_set_time_entry_tags(ctx,
							  [guid UTF8String],
							  value);
}

- (void)updateTimeEntryWithStartDate:(NSDate *)startDate
								guid:(NSString *)guid
{
	toggl_set_time_entry_date(ctx,
							  [guid UTF8String],
							  [startDate timeIntervalSince1970]);
}

- (void)updateTimeEntryWithDuration:(NSString *)duration
							   guid:(NSString *)guid
{
	toggl_set_time_entry_duration(ctx,
								  [guid UTF8String],
								  [duration UTF8String]);
}

- (void)updateTimeEntryWithStartTime:(NSString *)startTime
								guid:(NSString *)guid
{
	toggl_set_time_entry_start(ctx,
							   [guid UTF8String],
							   [startTime UTF8String]);
}

- (void)updateTimeEntryWithStartAtTimestamp:(NSTimeInterval)timestamp
									   guid:(NSString *)guid
{
	toggl_set_time_entry_start_timestamp(ctx,
										 [guid UTF8String],
										 timestamp);
}

- (void)updateTimeEntryWithEndTime:(NSString *)endTime
							  guid:(NSString *)guid
{
	toggl_set_time_entry_end(ctx,
							 [guid UTF8String],
							 [endTime UTF8String]);
}

- (void)updateTimeEntryWithEndAtTimestamp:(NSTimeInterval)timestamp
									 guid:(NSString *)guid
{
	toggl_set_time_entry_end_timestamp(ctx,
									   [guid UTF8String],
									   timestamp);
}

- (void)deleteTimeEntryImte:(TimeEntryViewItem *)item
{
	// If description is empty and duration is less than 15 seconds delete without confirmation
	if ([item confirmlessDelete])
	{
		toggl_delete_time_entry(ctx, [item.GUID UTF8String]);
		return;
	}
	NSString *msg = [NSString stringWithFormat:@"Delete time entry \"%@\"?", item.Description];

	NSAlert *alert = [[NSAlert alloc] init];
	[alert addButtonWithTitle:@"OK"];
	[alert addButtonWithTitle:@"Cancel"];
	[alert setMessageText:msg];
	[alert setInformativeText:@"Deleted time entries cannot be restored."];
	[alert setAlertStyle:NSWarningAlertStyle];
	if ([alert runModal] != NSAlertFirstButtonReturn)
	{
		return;
	}

	// Delete
	toggl_delete_time_entry(ctx, [item.GUID UTF8String]);
}

- (void)updateDescriptionForTimeEntry:(TimeEntryViewItem *)timeEntry autocomplete:(AutocompleteItem *)autocomplete
{
	toggl_set_time_entry_project(ctx,
								 [timeEntry.GUID UTF8String],
								 autocomplete.TaskID,
								 autocomplete.ProjectID,
								 0);
	toggl_set_time_entry_description(ctx,
									 [timeEntry.GUID UTF8String],
									 [autocomplete.Description UTF8String]);
	[self updateTimeEntryWithTags:autocomplete.tags guid:timeEntry.GUID];
}

- (NSString *)convertDuratonInSecond:(int64_t)durationInSecond
{
	char *str = toggl_format_tracking_time_duration(durationInSecond);
	NSString *newValue = [NSString stringWithUTF8String:str];

	free(str);
	return [newValue copy];
}

#pragma mark - Timeline

- (void)enableTimelineRecord:(BOOL)isEnabled
{
	toggl_timeline_toggle_recording(ctx, isEnabled);

    // Try to grant permission
    if (isEnabled)
    {
        [ObjcSystemPermissionManager tryGrantScreenRecordingPermission];
    }
}

- (void)timelineSetPreviousDate
{
	toggl_view_timeline_prev_day(ctx);
}

- (void)timelineSetNextDate
{
	toggl_view_timeline_next_day(ctx);
}

- (void)timelineSetDate:(NSDate *)date
{
	toggl_view_timeline_set_day(ctx, date.timeIntervalSince1970);
}

- (void)fetchTimelineData
{
	toggl_view_timeline_data(ctx);
}

- (void)timelineGetCurrentDate
{
	toggl_view_timeline_current_day(ctx);
}

- (NSString *)starNewTimeEntryAtStarted:(NSTimeInterval)started ended:(NSTimeInterval)ended
{
	char *guid = toggl_start(ctx,
							 "",
							 "",
							 0,
							 0,
							 0,
							 NULL,
							 false,
							 started,
							 ended
							 );
	NSString *GUID = [NSString stringWithUTF8String:guid];

	free(guid);
	return GUID;
}

- (void)startEditorAtGUID:(NSString *)GUID
{
	toggl_edit(ctx, [GUID UTF8String], false, kFocusedFieldNameDescription);
}

- (void)setEditorWindowSize:(CGSize)size
{
	toggl_set_window_edit_size_width(ctx, size.width);
	toggl_set_window_edit_size_height(ctx, size.height);
	track_edit_size(ctx, size.width, size.height);
}

- (CGSize)getEditorWindowSize
{
	NSInteger width = toggl_get_window_edit_size_width(ctx);
	NSInteger height = toggl_get_window_edit_size_height(ctx);

	// Prevent small size
	const CGSize minimumSize = CGSizeMake(274.0, 381.0);

	width = MAX(minimumSize.width, width);
	height = MAX(minimumSize.height, height);
	return CGSizeMake(width, height);
}

- (void)loadMoreTimeEntry
{
	toggl_load_more(ctx);
}

- (void)setClickCloseBtnInAppMessage
{
    toggl_iam_click(ctx, 2);
}

- (void)setClickActionBtnInAppMessage
{
    toggl_iam_click(ctx, 3);
}
@end
