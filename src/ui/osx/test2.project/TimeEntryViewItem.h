//
//  TimeEntryViewItem.h
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 25/09/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "kopsik_api.h"

@interface TimeEntryViewItem : NSObject
- (void)load:(KopsikTimeEntryViewItem *)data;
- (BOOL)isSameDay:(NSDate*)date;
+ (TimeEntryViewItem *)findByGUID:(NSString *)guid;
@property int duration_in_seconds;
@property NSString *Description; // uppercase to avoid clash with [NSObject description]
@property NSString *project;
@property NSString *duration;
@property NSString *color;
@property NSString *GUID;
@property BOOL billable;
@property NSDate *started;
@property NSDate *ended;
@property NSArray *tags;
@property NSString *updatedAt;
@property NSString *dateHeader;
@end
