//
//  TimeEntryViewItem.m
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 25/09/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "TimeEntryViewItem.h"

@implementation TimeEntryViewItem

- (void)load:(TogglTimeEntryView *)te
{
	self.GUID = [NSString stringWithUTF8String:te->GUID];
	self.duration_in_seconds = te->DurationInSeconds;
	if (te->Description)
	{
		self.Description = [NSString stringWithUTF8String:te->Description];
	}
	else
	{
		self.Description = @"";
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

	self.isHeader = NO;
	if (te->IsHeader)
	{
		self.isHeader = YES;
	}

	self.durOnly = NO;
	if (te->DurOnly)
	{
		self.durOnly = YES;
	}

	self.CanAddProjects = te->CanAddProjects;
	self.DefaultWID = te->DefaultWID;
	self.CanSeeBillable = te->CanSeeBillable;
	self.WorkspaceName = [NSString stringWithUTF8String:te->WorkspaceName];
	if (te->Error)
	{
		self.Error = [NSString stringWithUTF8String:te->Error];
	}
}

- (NSString *)description
{
	return [NSString stringWithFormat:@"GUID: %@, description: %@, started: %@, ended: %@, project: %@, seconds: %lld, duration: %@, color: %@, billable: %i, tags: %@",
			self.GUID, self.Description, self.started, self.ended,
			self.ProjectAndTaskLabel, self.duration_in_seconds, self.duration,
			self.ProjectColor, self.billable, self.tags];
}

@end
