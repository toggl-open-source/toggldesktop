//
//  TimelineEventsListItem.h
//  TogglDesktop
//
//  Created by Indrek Vändrik on 05/02/16.
//  Copyright © 2016 Alari. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "TimelineChunkView.h"
#import "TimelineChunkCircle.h"

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
@end
