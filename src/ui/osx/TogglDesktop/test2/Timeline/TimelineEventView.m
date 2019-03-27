//
//  TimelineEventView.m
//  TogglDesktop
//
//  Created by Indrek Vändrik on 27/03/2019.
//  Copyright © 2019 Toggl Desktop Developers. All rights reserved.
//

#import "TimelineEventView.h"
#import "Utils.h"

@implementation TimelineEventView

- (void)load:(TogglTimelineEventView *)view
{
	self.Header = view->Header;
	self.Title = [NSString stringWithUTF8String:view->Title];
	self.Filename = [NSString stringWithUTF8String:view->Filename];
	self.Duration = (int)view->Duration;
	[self updateSorter];
}

- (void)updateSorter
{
	self.Sorter = [NSString stringWithFormat:@"%@%@%d", [self.Filename lowercaseString], [self.Title lowercaseString], self.Duration];
}

- (NSString *)prettyDuration
{
	return [Utils formatTimeFromSeconds:self.Duration];
}

- (NSMutableAttributedString *)descriptionString
{
	NSString *str;
	NSMutableAttributedString *strm;

	if (self.Header)
	{
		str = [NSString stringWithFormat:@"%@\t%@\n", self.Filename, [self prettyDuration]];
		strm = [[NSMutableAttributedString alloc] initWithString:str];
		[strm setAttributes:@{ NSFontAttributeName : [NSFont boldSystemFontOfSize:[NSFont systemFontSize]] } range:NSMakeRange(0, [str length])];
	}
	else
	{
		str = [NSString stringWithFormat:@"\t%@\t%@\n", self.Title, [self prettyDuration]];
		strm = [[NSMutableAttributedString alloc] initWithString:str];
	}

	return strm;
}

- (NSString *)description
{
	return [NSString stringWithFormat:@"Title: %@, Filename: %@, Duration: %d",
			self.Title, self.Filename, self.Duration];
}

@end
