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
@property uint64_t Started;
@property (weak) IBOutlet NSTextField *timeLabel;
@property (weak) IBOutlet NSBox *appsBox;
@property (weak) IBOutlet NSTextField *selectedLabel;
@property (weak) IBOutlet NSBox *backgroundBox;
@property (weak) IBOutlet NSImageView *arrowImage;
@property (unsafe_unretained) IBOutlet NSTextView *appTitleTextView;
@property NSInteger rowIndex;
@property (weak) IBOutlet TimelineChunkCircle *activityCircle;
@end
