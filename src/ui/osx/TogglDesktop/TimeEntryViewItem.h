//
//  TimeEntryViewItem.h
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 25/09/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "toggl_api.h"

@class NSColor;

@interface TimeEntryViewItem : NSObject
@property (assign, nonatomic) int64_t duration_in_seconds;
@property (copy, nonatomic) NSString *Description; // uppercase to avoid clash with [NSObject description]
@property (copy, nonatomic) NSString *descriptionName;
@property (copy, nonatomic) NSString *ProjectAndTaskLabel;
@property (copy, nonatomic) NSString *TaskLabel;
@property (copy, nonatomic) NSString *ProjectLabel;
@property (copy, nonatomic) NSString *ClientLabel;
@property (assign, nonatomic) uint64_t WorkspaceID;
@property (assign, nonatomic) uint64_t ProjectID;
@property (assign, nonatomic) uint64_t TaskID;
@property (copy, nonatomic) NSString *duration;
@property (copy, nonatomic) NSString *ProjectColor;
@property (copy, nonatomic) NSString *ProjectGUID; // when project is not saved to backend yet
@property (copy, nonatomic) NSString *GUID;
@property (assign, nonatomic) BOOL billable;
@property (assign, nonatomic) BOOL unsynced;
@property (strong, nonatomic) NSDate *started;
@property (strong, nonatomic) NSDate *ended;
@property (copy, nonatomic) NSString *startTimeString;
@property (copy, nonatomic) NSString *endTimeString;
@property (strong, nonatomic) NSArray *tags;
@property (strong, nonatomic) NSDate *updatedAt;
@property (copy, nonatomic) NSString *formattedDate;
// If this is set to YES, a date header is displayed on top of
// the time entry cell in the time entry list view:
@property (copy, nonatomic) NSNumber *isHeader;
// If it's a header, this is useful:
@property (copy, nonatomic) NSString *dateDuration;
// If TE is passed for editing, this is the field that should be focussed
@property (copy, nonatomic) NSString *focusedFieldName;
@property (assign, nonatomic) BOOL CanSeeBillable;
@property (assign, nonatomic) uint64_t DefaultWID;
@property (assign, nonatomic) BOOL CanAddProjects;
@property (copy, nonatomic) NSString *WorkspaceName;
@property (copy, nonatomic) NSString *Error;
// if the item is load more row
@property (assign, nonatomic) BOOL loadMore;
// Grouped Mode attributes
// If entry is group header
@property (assign, nonatomic) BOOL Group;
// Group Open state
@property (assign, nonatomic) BOOL GroupOpen;
// Group Name
@property (copy, nonatomic) NSString *GroupName;
// Group Duration
@property (copy, nonatomic) NSString *GroupDuration;
// Group Item Count
@property (assign, nonatomic) uint64_t GroupItemCount;
@property (copy, nonatomic) NSString *touchBarTitle;

- (NSString *)descriptionEntry;
// if item can be deleted without confirm
- (BOOL)confirmlessDelete;
- (BOOL)isRunning;
+ (NSColor *)defaultProjectColor;
- (void)load:(TogglTimeEntryView *)data;

@end
