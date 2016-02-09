//
//  TimelineEventView.m
//  TogglDesktop
//
//  Created by Tanel Lebedev on 26/10/15.
//  Copyright © 2015 Toggl Desktop Developers. All rights reserved.
//

#import "TimelineEventView.h"

@implementation TimelineEventView

- (void)load:(TogglTimelineEventView *)view
{
	self.Title = [NSString stringWithUTF8String:view->Title];
	self.Filename = [NSString stringWithUTF8String:view->Filename];
	self.Duration = view->Duration;
}

- (NSMutableAttributedString *)descriptionString
{
	NSString *str;
	NSMutableAttributedString *strm;

	if ([self.Title length] > 0)
	{
		str = [NSString stringWithFormat:@"ITEM | %@ -- %@ %lld\n", self.Filename, self.Title, self.Duration];
		strm = [[NSMutableAttributedString alloc] initWithString:str];
	}
	else
	{
		str = [NSString stringWithFormat:@"CATEGORY | %@ -- %@ %lld\n", self.Filename, self.Title, self.Duration];
		strm = [[NSMutableAttributedString alloc] initWithString:str];
		[strm setAttributes:@{ NSFontAttributeName : [NSFont boldSystemFontOfSize:[NSFont systemFontSize]] } range:NSMakeRange(0, [str length])];
	}

	return strm;
}

- (NSString *)description
{
	return [NSString stringWithFormat:@"Title: %@, Filename: %@, Duration: %lld",
			self.Title, self.Filename, self.Duration];
}

@end
