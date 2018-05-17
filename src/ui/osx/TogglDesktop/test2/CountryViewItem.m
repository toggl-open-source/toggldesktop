//
//  CountryViewItem.m
//  TogglDesktop
//
//  Created by Indrek Vändrik on 16/05/2018.
//  Copyright © 2018 Alari. All rights reserved.
//

#import "CountryViewItem.h"

@implementation CountryViewItem

- (void)load:(TogglCountryView *)data
{
	self.ID = data->ID;
	self.VatApplicable = data->VatApplicable;
	self.Text = [NSString stringWithUTF8String:data->Name];
	self.Name = [NSString stringWithUTF8String:data->Name];
	self.VatPercentage = [NSString stringWithUTF8String:data->VatPercentage];
	self.VatRegex = [NSString stringWithUTF8String:data->VatRegex];
	self.Code = [NSString stringWithUTF8String:data->Code];
}

+ (NSMutableArray *)loadAll:(TogglCountryView *)first
{
	NSMutableArray *result = [[NSMutableArray alloc] init];
	TogglCountryView *it = first;

	while (it)
	{
		CountryViewItem *item = [[CountryViewItem alloc] init];
		[item load:it];
		[result addObject:item];
		it = it->Next;
	}
	return result;
}

@end
