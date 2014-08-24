//
//  AutocompleteItem.m
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 18/11/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "AutocompleteItem.h"

@implementation AutocompleteItem

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
	}
	else
	{
		self.Description = @"";
	}

	if (data->ProjectColor)
	{
		self.ProjectColor = [NSString stringWithUTF8String:data->ProjectColor];
	}
	else
	{
		self.ProjectColor = @"";
	}
	self.Type = data->Type;
	self.ProjectID = data->ProjectID;
	self.TaskID = data->TaskID;
}

- (void)save:(TogglAutocompleteView *)data
{
	NSAssert(!data->Text, @"data already has text");
	data->Text = strdup([self.Text UTF8String]);
	data->Type = (unsigned int)self.Type;
	data->ProjectID = (unsigned int)self.ProjectID;
	data->TaskID = (unsigned int)self.TaskID;
}

- (NSString *)description
{
	return [NSString stringWithFormat:@"Text: %@, PID: %llu, TID: %llu, type: %llu",
			self.Text, self.ProjectID, self.TaskID, self.Type];
}

+ (NSMutableArray *)loadAll:(TogglAutocompleteView *)first
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
	return result;
}

@end
