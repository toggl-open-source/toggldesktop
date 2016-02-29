//
//  TimelineViewController.h
//  TogglDesktop
//
//  Created by Tanel Lebedev on 22/10/15.
//  Copyright © 2015 Toggl Desktop Developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "toggl_api.h"
#import "NSUnstripedTableView.h"
#import "TimelineEventsListItem.h"

@interface TimelineViewController : NSViewController <NSTableViewDelegate, NSTableViewDataSource> {
	@private
	NSMutableArray *timelineChunks;
	NSMutableArray *highlightedRows;
}
@property IBOutlet NSTextField *dateLabel;
@property IBOutlet NSTextField *timeLabel;
@property IBOutlet NSTextField *descriptionText;
@property IBOutlet NSButton *prevButton;
@property IBOutlet NSButton *nextButton;
@property IBOutlet NSButton *createButton;
@property (strong) IBOutlet NSUnstripedTableView *eventsTableView;
@property (strong) IBOutlet NSScrollView *eventsBox;
@property (strong) IBOutlet NSBox *headerBox;
@property (strong) IBOutlet NSBox *footerBox;
@property BOOL startTimeSet;
@property NSInteger lastRow;
@property TimelineEventsListItem *startItem;
@property TimelineEventsListItem *endItem;
- (IBAction)prevButtonClicked:(id)sender;
- (IBAction)nextButtonClicked:(id)sender;
- (IBAction)createButtonClicked:(id)sender;
- (void)setHighlights;
@property (weak) IBOutlet NSTextField *startTimeLabel;
@property (weak) IBOutlet NSTextField *endTimeLabel;

@end
