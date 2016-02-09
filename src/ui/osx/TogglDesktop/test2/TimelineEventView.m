//
//  TimelineEventView.m
//  TogglDesktop
//
//  Created by Tanel Lebedev on 26/10/15.
//  Copyright © 2015 Toggl Desktop Developers. All rights reserved.
//

#import "TimelineEventView.h"
#import "Utils.h"

@implementation TimelineEventView

- (void)load:(TogglTimelineEventView *)view
{
	self.Title = [NSString stringWithUTF8String:view->Title];
	self.Filename = [NSString stringWithUTF8String:view->Filename];
	self.Duration = (int)view->Duration;
}

- (void)updateSorter
{
	self.Sorter = [NSString stringWithFormat:@"%@%@", [self.Filename lowercaseString], [self.Title lowercaseString]];
}

- (NSString *)prettyDuration
{
	return [Utils formatTimeFromSeconds:self.Duration];
}

- (NSMutableAttributedString *)descriptionString
{
	NSString *str;
	NSMutableAttributedString *strm;

	if ([self.Title length] > 0)
	{
		str = [NSString stringWithFormat:@"\t%@\t%@\n", self.Title, [self prettyDuration]];
		strm = [[NSMutableAttributedString alloc] initWithString:str];
	}
	else
	{
		str = [NSString stringWithFormat:@"%@\t%@\n", self.Filename, [self prettyDuration]];
		strm = [[NSMutableAttributedString alloc] initWithString:str];
		[strm setAttributes:@{ NSFontAttributeName : [NSFont boldSystemFontOfSize:[NSFont systemFontSize]] } range:NSMakeRange(0, [str length])];
	}

	return strm;
}

- (NSString *)description
{
	return [NSString stringWithFormat:@"Title: %@, Filename: %@, Duration: %d",
			self.Title, self.Filename, self.Duration];
}

@end
