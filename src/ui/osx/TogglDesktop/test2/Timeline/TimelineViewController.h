//
//  TimelineViewController.h
//  TogglDesktop
//
//  Created by Indrek Vändrik on 27/03/2019.
//  Copyright © 2019 Toggl Desktop Developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "toggl_api.h"
// #import "NSUnstripedTableView.h"
#import "TimelineEventsListItem.h"
#import "TDBarChart.h"

NS_ASSUME_NONNULL_BEGIN

@interface TimelineViewController : NSViewController <NSTableViewDelegate, NSTableViewDataSource>
{
	@private
	NSMutableArray *timelineChunks;
	NSMutableArray *highlightedRows;
}
@property (weak) IBOutlet TDBarChart *barChart;
@property IBOutlet NSTextField *dateLabel;
@property IBOutlet NSTextField *timeLabel;
@property IBOutlet NSTextField *descriptionText;
@property IBOutlet NSButton *prevButton;
@property IBOutlet NSButton *nextButton;
@property IBOutlet NSButton *createButton;
@property (strong) IBOutlet NSTableView *eventsTableView;
@property (strong) IBOutlet NSScrollView *eventsBox;
@property (strong) IBOutlet NSBox *headerBox;
@property (strong) IBOutlet NSBox *footerBox;
@property BOOL startTimeSet;
@property BOOL today;
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

NS_ASSUME_NONNULL_END
