//
//  ViewItem.m
//  kopsik_ui_osx
//
//  Copyright (c) 2014 Toggl Desktop developers. All rights reserved.
//

#import "ViewItem.h"

@implementation ViewItem

- (void)load:(KopsikViewItem *)data
{
	self.ID = data->ID;
  self.WID = data->WID;
	self.Name = [NSString stringWithUTF8String:data->Name];
	self.GUID = nil;
	if (data->GUID)
	{
		self.GUID = [NSString stringWithUTF8String:data->GUID];
	}
}

+ (NSMutableArray *)loadAll:(KopsikViewItem *)first
{
	NSMutableArray *result = [[NSMutableArray alloc] init];
	KopsikViewItem *it = first;

	while (it)
	{
		ViewItem *item = [[ViewItem alloc] init];
		[item load:it];
		[result addObject:item];
		it = it->Next;
	}
	return result;
}

@end
