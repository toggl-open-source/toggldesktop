//
//  TimelineEventsListItem.m
//  TogglDesktop
//
//  Created by Indrek Vändrik on 05/02/16.
//  Copyright © 2016 Alari. All rights reserved.
//

#import "TimelineEventsListItem.h"
#import "TimelineEventView.h"

@implementation TimelineEventsListItem

- (void)drawRect:(NSRect)dirtyRect
{
	[super drawRect:dirtyRect];

	// Drawing code here.
}

- (void)render:(TimelineChunkView *)view_item
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	NSString *events = @"";

	self.timeLabel.stringValue = view_item.StartTimeString;
	for (TimelineEventView *event in view_item.Events)
	{
        events = [events stringByAppendingString:event.Filename];
		events = [events stringByAppendingString:@" - "];
        events = [events stringByAppendingString:event.Title];
        events = [events stringByAppendingString:@" - "];
        events = [events stringByAppendingString:[NSString stringWithFormat:@"%lld",event.Duration]];
		events = [events stringByAppendingString:@"\n"];
	}
	NSMutableAttributedString *muAtrStr = [[NSMutableAttributedString alloc]initWithString:events];
	[[self.appTitlesTextView textStorage] setAttributedString:muAtrStr];

	[self.appsBox setHidden:[events length] == 0];
}

@end
