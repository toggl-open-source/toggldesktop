//
//  TimeEntryViewItem.m
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 25/09/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "TimeEntryViewItem.h"
#import "TogglDesktop-Swift.h"

@implementation TimeEntryViewItem

- (void)load:(TogglTimeEntryView *)te
{
	self.GUID = [NSString stringWithUTF8String:te->GUID];
	self.duration_in_seconds = te->DurationInSeconds;
	if (te->Description)
	{
		self.Description = [NSString stringWithUTF8String:te->Description];
		self.descriptionName = [NSString stringWithUTF8String:te->Description];
	}
	else
	{
		self.Description = @"";
		self.descriptionName = @"";
	}
	if (te->ProjectAndTaskLabel)
	{
		self.ProjectAndTaskLabel = [NSString stringWithUTF8String:te->ProjectAndTaskLabel];
		self.TaskLabel = [NSString stringWithUTF8String:te->TaskLabel];
		self.ProjectLabel = [NSString stringWithUTF8String:te->ProjectLabel];
		self.ClientLabel = [NSString stringWithUTF8String:te->ClientLabel];
	}
	else
	{
		self.ProjectAndTaskLabel = nil;
		self.TaskLabel = nil;
		self.ProjectLabel = nil;
		self.ClientLabel = nil;
	}
	self.WorkspaceID = te->WID;
	self.ProjectID = te->PID;
	self.TaskID = te->TID;
	if (te->Color)
	{
		self.ProjectColor = [NSString stringWithUTF8String:te->Color];
	}
	else
	{
		self.ProjectColor = nil;
	}
	self.duration = [NSString stringWithUTF8String:te->Duration];
	if (te->Tags)
	{
		NSString *tagList = [NSString stringWithUTF8String:te->Tags];
		self.tags = [tagList componentsSeparatedByString:@"\t"];
	}
	else
	{
		self.tags = nil;
	}
	if (te->Billable)
	{
		self.billable = YES;
	}
	else
	{
		self.billable = NO;
	}
	if (te->Unsynced)
	{
		self.unsynced = YES;
	}
	else
	{
		self.unsynced = NO;
	}
	self.started = [NSDate dateWithTimeIntervalSince1970:te->Started];
	self.ended = [NSDate dateWithTimeIntervalSince1970:te->Ended];
	if (te->StartTimeString)
	{
		self.startTimeString = [NSString stringWithUTF8String:te->StartTimeString];
	}
	else
	{
		self.startTimeString = nil;
	}
	if (te->EndTimeString)
	{
		self.endTimeString = [NSString stringWithUTF8String:te->EndTimeString];
	}
	else
	{
		self.endTimeString = nil;
	}
	if (te->UpdatedAt)
	{
		self.updatedAt = [NSDate dateWithTimeIntervalSince1970:te->UpdatedAt];
	}
	else
	{
		self.updatedAt = nil;
	}
	self.formattedDate = [NSString stringWithUTF8String:te->DateHeader];
	if (te->DateDuration)
	{
		self.dateDuration = [NSString stringWithUTF8String:te->DateDuration];
	}
	else
	{
		self.dateDuration = nil;
	}

	self.isHeader = @NO;
	if (te->IsHeader)
	{
		self.isHeader = @YES;
	}

	self.CanAddProjects = te->CanAddProjects;
	self.DefaultWID = te->DefaultWID;
	self.CanSeeBillable = te->CanSeeBillable;
	if (te->WorkspaceName)
	{
		self.WorkspaceName = [NSString stringWithUTF8String:te->WorkspaceName];
	}
	else
	{
		self.WorkspaceName = nil;
	}
	if (te->Error)
	{
		self.Error = [NSString stringWithUTF8String:te->Error];
	}
	else
	{
		self.Error = nil;
	}

	// Grouped mode

	// If this entry is group header
	self.Group = NO;
	if (te->Group)
	{
		self.Group = YES;
	}
	self.GroupOpen = NO;
	if (te->GroupOpen)
	{
		self.GroupOpen = YES;
	}
	self.GroupName = [NSString stringWithUTF8String:te->GroupName];
	self.GroupDuration = [NSString stringWithUTF8String:te->GroupDuration];
	self.GroupItemCount = te->GroupItemCount;

	if (self.descriptionName.length != 0)
	{
		self.touchBarTitle = self.descriptionName;
	}
	else
	{
		if (self.ProjectLabel.length != 0)
		{
			self.touchBarTitle = self.ProjectLabel;
		}
		else
		{
			self.touchBarTitle = @"(no description)";
		}
	}

	[[UndoManager shared] storeWith:self];
}

- (void)setLoadMore
{
	self.loadMore = YES;
}

- (NSString *)description
{
	return [NSString stringWithFormat:@"Address: %@, GUID: %@, description: %@, started: %@, ended: %@, project: %@, seconds: %lld, duration: %@, color: %@, billable: %i, tags: %@", [super description],
			self.GUID, self.Description, self.started, self.ended,
			self.ProjectAndTaskLabel, self.duration_in_seconds, self.duration,
			self.ProjectColor, self.billable, self.tags];
}

- (NSUInteger)hash {
	return self.duration_in_seconds ^ self.Description.hash ^ self.ProjectAndTaskLabel.hash ^ self.TaskLabel.hash ^ self.ProjectLabel.hash ^ self.ClientLabel.hash ^ self.WorkspaceID ^ self.ProjectID ^ self.TaskID ^ self.duration.hash ^ self.ProjectColor.hash ^ self.ProjectGUID.hash ^ self.GUID.hash ^ self.billable ^ self.unsynced ^ self.started.hash ^ self.ended.hash ^ self.startTimeString.hash ^ self.endTimeString.hash ^ self.tags.hash ^ self.updatedAt.hash ^ self.formattedDate.hash ^ self.isHeader.hash ^ self.dateDuration.hash ^ self.focusedFieldName.hash ^ self.CanSeeBillable ^ self.DefaultWID ^ self.CanAddProjects ^ self.WorkspaceName.hash ^ self.Error.hash ^ self.loadMore ^ self.confirmlessDelete ^ self.Group ^ self.GroupOpen ^ self.GroupName.hash ^ self.GroupDuration.hash ^ self.GroupItemCount;
}

- (NSString *)descriptionEntry {
	return self.Description;
}

- (BOOL)confirmlessDelete
{
	if (self.duration_in_seconds < 0)
	{
		int64_t actual_duration = self.duration_in_seconds + time(0);
		return actual_duration < 15;
	}
	else
	{
		return self.duration_in_seconds < 15;
	}
}

- (BOOL)isRunning
{
	return self.duration_in_seconds < 0;
}

- (BOOL) isSameContentWithTimeEntryViewItem:(TimeEntryViewItem *) item
{
    return [self.descriptionName isEqualToString:item.descriptionName] &&
    [self.ProjectAndTaskLabel isEqualToString:item.ProjectAndTaskLabel] &&
    [self.ProjectColor isEqualToString:item.ProjectColor];
}
@end
