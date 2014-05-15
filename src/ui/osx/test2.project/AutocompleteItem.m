//
//  AutocompleteItem.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 18/11/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "AutocompleteItem.h"

@implementation AutocompleteItem

- (void)load:(KopsikAutocompleteItem *)data
{
	self.Text = [NSString stringWithUTF8String:data->Text];
	self.ProjectAndTaskLabel = [NSString stringWithUTF8String:data->ProjectAndTaskLabel];
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

- (void)save:(KopsikAutocompleteItem *)data
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

@end
