//
//  TimelineEventsListItem.h
//  TogglDesktop
//
//  Created by Indrek Vändrik on 27/03/2019.
//  Copyright © 2019 Toggl Desktop Developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "TimelineChunkView.h"
#import "TimelineChunkCircle.h"
#import "TDBarChart.h"
#import "TimelineLines.h"

NS_ASSUME_NONNULL_BEGIN

@interface TimelineEventsListItem : NSTableCellView
- (void)render:(TimelineChunkView *)view_item;
- (void)setSelected:(BOOL)endTime row:(NSInteger)rowIndex;
- (void)setUnSelected;
- (void)setActive:(BOOL)edge;
@property uint64_t Started;
@property NSInteger rowIndex;
@property (weak) IBOutlet NSTextField *timeLabel;
@property (weak) IBOutlet NSBox *appsBox;
@property (weak) IBOutlet NSTextField *selectedLabel;
@property (weak) IBOutlet NSBox *backgroundBox;
@property (unsafe_unretained) IBOutlet NSTextView *appTitleTextView;
@property (weak) IBOutlet TimelineChunkCircle *activityCircle;
@property (weak) IBOutlet NSBox *entryLine;
@property NSLayoutConstraint *topConstraint;
@property NSLayoutConstraint *bottomConstraint;
@property (weak) IBOutlet TDBarChart *barChart;
@property (weak) IBOutlet TimelineLines *lines;
@end

NS_ASSUME_NONNULL_END
