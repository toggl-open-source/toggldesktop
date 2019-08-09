//
//  AutocompleteItem.m
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 18/11/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "AutocompleteItem.h"
#import "TogglDesktop-Swift.h"

@implementation AutocompleteItem

- (instancetype)initWithSnapshot:(ProjectSnapshot *)snapshot
{
	self = [super init];
	if (self)
	{
		self.ProjectAndTaskLabel = snapshot.projectAndTaskLabel;
		self.TaskID = snapshot.taskID;
		self.ProjectID = snapshot.projectID;
		self.ProjectLabel = snapshot.projectLabel;
		self.ProjectColor = snapshot.projectColor;
	}
	return self;
}

- (void)load:(TogglAutocompleteView *)data
{
	self.Text = [NSString stringWithUTF8String:data->Text];
	self.ProjectAndTaskLabel = [NSString stringWithUTF8String:data->ProjectAndTaskLabel];
	if (data->TaskLabel)
	{
		self.TaskLabel = [NSString stringWithUTF8String:data->TaskLabel];
	}
	else
	{
		self.TaskLabel = @"";
	}

	if (data->ProjectLabel)
	{
		self.ProjectLabel = [NSString stringWithUTF8String:data->ProjectLabel];
	}
	else
	{
		self.ProjectLabel = @"";
	}

	if (data->ClientLabel)
	{
		self.ClientLabel = [NSString stringWithUTF8String:data->ClientLabel];
	}
	else
	{
		self.ClientLabel = @"";
	}

	if (data->Description)
	{
		self.Description = [NSString stringWithUTF8String:data->Description];
		self.DescriptionTitle = [NSString stringWithUTF8String:data->Description];
	}
	else
	{
		self.Description = @"";
		self.DescriptionTitle = @"";
	}

	if (data->ProjectColor)
	{
		self.ProjectColor = [NSString stringWithUTF8String:data->ProjectColor];
	}
	else
	{
		self.ProjectColor = @"";
	}
	if (data->WorkspaceName)
	{
		self.WorkspaceName = [NSString stringWithUTF8String:data->WorkspaceName];
	}
	else
	{
		self.WorkspaceName = nil;
	}
	self.Type = data->Type;
	self.WorkspaceID = data->WorkspaceID;
	self.ProjectID = data->ProjectID;
	self.TaskID = data->TaskID;
	if (data->Tags)
	{
		NSString *tagList = [NSString stringWithUTF8String:data->Tags];
		self.tags = [tagList componentsSeparatedByString:@"\t"];
	}
	else
	{
		self.tags = nil;
	}
	if (data->ProjectGUID)
	{
		self.ProjectGUID = [NSString stringWithUTF8String:data->ProjectGUID];
	}
	else
	{
		self.ProjectGUID = @"";
	}

	if (data->Billable)
	{
		self.Billable = data->Billable;
	}
}

- (void)save:(TogglAutocompleteView *)data
{
	NSAssert(!data->Text, @"data already has text");
	data->Text = strdup([self.Text UTF8String]);
	data->Type = (unsigned int)self.Type;
	data->WorkspaceID = (unsigned int)self.WorkspaceID;
	data->ProjectID = (unsigned int)self.ProjectID;
	data->TaskID = (unsigned int)self.TaskID;
	data->Tags = strdup([[self.tags componentsJoinedByString:@"\t"] UTF8String]);
}

+ (NSArray<AutocompleteItem *> *)loadAll:(TogglAutocompleteView *)first
{
	NSMutableArray *result = [[NSMutableArray alloc] init];
	TogglAutocompleteView *record = first;

	while (record)
	{
		AutocompleteItem *item = [[AutocompleteItem alloc] init];
		[item load:record];
		[result addObject:item];
		record = record->Next;
	}
	return [result copy];
}

@end
