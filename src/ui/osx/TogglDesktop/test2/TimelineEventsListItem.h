//
//  TimelineEventsListItem.h
//  TogglDesktop
//
//  Created by Indrek Vändrik on 05/02/16.
//  Copyright © 2016 Alari. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "TimelineChunkView.h"

@interface TimelineEventsListItem : NSTableCellView
- (void)render:(TimelineChunkView *)view_item;
- (void)setSelected:(BOOL)endTime;
- (void)setUnSelected;
@property uint64_t Started;
@property (weak) IBOutlet NSTextField *timeLabel;
@property (weak) IBOutlet NSBox *appsBox;
@property (unsafe_unretained) IBOutlet NSTextView *appTitlesTextView;
@property (weak) IBOutlet NSTextField *selectedLabel;
@end
