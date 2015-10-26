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
}

- (NSString *)description
{
	return [NSString stringWithFormat:@"Title: %@, Filename: %@",
			self.Title, self.Filename];
}

@end
