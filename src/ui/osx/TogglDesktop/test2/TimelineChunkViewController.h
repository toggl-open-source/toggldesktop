//
//  TimelineChunkViewController.h
//  TogglDesktop
//
//  Created by Tanel Lebedev on 26/10/15.
//  Copyright © 2015 Toggl Desktop Developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface TimelineChunkViewController : NSViewController {
	@private
	NSMutableArray *timelineEvents;
}
@property IBOutlet NSTextField *startLabel;
@property IBOutlet NSButton *expandButton;
@property IBOutlet NSTableView *eventsTableView;
@end
