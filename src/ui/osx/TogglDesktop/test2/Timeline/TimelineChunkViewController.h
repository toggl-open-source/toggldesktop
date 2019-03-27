//
//  TimelineChunkViewController.h
//  TogglDesktop
//
//  Created by Indrek Vändrik on 27/03/2019.
//  Copyright © 2019 Toggl Desktop Developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

@interface TimelineChunkViewController : NSViewController
{
	@private
	NSMutableArray *timelineEvents;
}
@property IBOutlet NSTextField *startLabel;
@property IBOutlet NSButton *expandButton;
@property IBOutlet NSTableView *eventsTableView;
@end

NS_ASSUME_NONNULL_END
