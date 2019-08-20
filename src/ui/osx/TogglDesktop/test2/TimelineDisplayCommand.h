//
//  TimelineDisplayCommand.h
//  TogglDesktop
//
//  Created by Nghia Tran on 6/21/19.
//  Copyright © 2019 Alari. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "toggl_api.h"

NS_ASSUME_NONNULL_BEGIN

@class TimeEntryViewItem;

@interface TimelineDisplayEvent : NSObject
@property (copy, nonatomic) NSString *title;
@property (copy, nonatomic) NSString *fileName;
@property (assign, nonatomic) NSTimeInterval duration;
@property (assign, nonatomic) BOOL isHeader;
@property (strong, nonatomic) NSArray<TimelineDisplayEvent *> *subEvents;
@end

@interface TimelineDisplayActivity : NSObject
@property (assign, nonatomic) NSTimeInterval started;
@property (copy, nonatomic) NSString *startedTimeString;
@property (strong, nonatomic) NSArray<TimelineDisplayEvent * > *events;
@property (assign, nonatomic) NSTimeInterval duration;
@end

@interface TimelineDisplayCommand : NSObject

- (instancetype)initWithOpen:(BOOL)open
						date:(NSString *)date
			   firstActivity:(TogglTimelineChunkView *)firstActivity
				  firstEntry:(TogglTimeEntryView *)firstEntry
					startDay:(NSTimeInterval)startDay
					  endDay:(NSTimeInterval)endDay;

@property (assign, nonatomic) BOOL open;
@property (assign, nonatomic) NSTimeInterval start;
@property (assign, nonatomic) NSTimeInterval end;
@property (strong, nonatomic) NSArray<TimeEntryViewItem *> *timeEntries;
@property (strong, nonatomic) NSArray<TimelineDisplayActivity *> *activities;
@property (copy, nonatomic) NSString *timelineDate;

@end

NS_ASSUME_NONNULL_END
