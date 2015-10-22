//
//  TimelineViewController.m
//  TogglDesktop
//
//  Created by Tanel Lebedev on 22/10/15.
//  Copyright © 2015 Toggl Desktop Developers. All rights reserved.
//

#import "TimelineViewController.h"

#import "DisplayCommand.h"
#import "UIEvents.h"

@interface TimelineViewController ()

@end

@implementation TimelineViewController

extern void *ctx;

- (void)viewDidLoad
{
	[super viewDidLoad];

	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(startDisplayTimeline:)
												 name:kDisplayTimeline
											   object:nil];
}

- (IBAction)prevButtonClicked:(id)sender
{
	toggl_view_timeline_prev_day(ctx);
}

- (IBAction)nextButtonClicked:(id)sender
{
	toggl_view_timeline_next_day(ctx);
}

- (IBAction)createButtonClicked:(id)sender
{
	// FIXME: create time entry from the given timeline event range
}

- (void)startDisplayTimeline:(NSNotification *)notification
{
	[self performSelectorOnMainThread:@selector(displayTimeline:)
						   withObject:notification.object
						waitUntilDone:NO];
}

- (void)displayTimeline:(DisplayCommand *)cmd
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");
	if (cmd.open)
	{
		self.dateLabel.stringValue = [NSString stringWithFormat:@"Timeline %@", cmd.timelineDate];

		// FIXME: reload list
	}
}

@end
