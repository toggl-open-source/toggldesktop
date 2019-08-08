//
//  TimelineChunkView.h
//  TogglDesktop
//
//  Created by Indrek Vändrik on 27/03/2019.
//  Copyright © 2019 Toggl Desktop Developers. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "toggl_api.h"

NS_ASSUME_NONNULL_BEGIN

@class TimeEntryViewItem;
@class TimelineEventView;

@interface TimelineChunkView : NSObject
@property (assign, nonatomic) uint64_t Started;
@property (assign, nonatomic) NSInteger activeDuration;
@property (assign, nonatomic) uint64_t EntryStart;
@property (assign, nonatomic) uint64_t EntryEnd;
@property (assign, nonatomic) uint64_t CalculatedHeight;
@property (strong, nonatomic) NSString *StartTimeString;
@property (strong, nonatomic) NSArray<TimelineEventView *> *Events;
@property (strong, nonatomic) NSArray<TimeEntryViewItem *> *Entries;
@property (copy, nonatomic) NSString *EntryDescription;

- (void)load:(TogglTimelineChunkView *)data;

@end

NS_ASSUME_NONNULL_END
