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

@interface TimelineDisplayCommand : NSObject

- (instancetype)initWithOpen:(BOOL)open
						date:(NSString *)date
				  firstEntry:(TogglTimeEntryView *)firstEntry
					startDay:(NSTimeInterval)startDay
					  endDay:(NSTimeInterval)endDay;

@property (assign, nonatomic) BOOL open;
@property (assign, nonatomic) NSTimeInterval start;
@property (assign, nonatomic) NSTimeInterval end;
@property (strong, nonatomic) NSArray<TimeEntryViewItem *> *timeEntries;
@property (copy, nonatomic) NSString *timelineDate;

@end

NS_ASSUME_NONNULL_END
