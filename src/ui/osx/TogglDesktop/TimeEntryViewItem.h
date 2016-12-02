//
//  TimeEntryViewItem.h
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 25/09/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "toggl_api.h"

@interface TimeEntryViewItem : NSObject
- (void)load:(TogglTimeEntryView *)data;
@property int64_t duration_in_seconds;
@property (strong) NSString *Description; // uppercase to avoid clash with [NSObject description]
@property (strong) NSString *ProjectAndTaskLabel;
@property (strong) NSString *TaskLabel;
@property (strong) NSString *ProjectLabel;
@property (strong) NSString *ClientLabel;
@property uint64_t WorkspaceID;
@property uint64_t ProjectID;
@property uint64_t TaskID;
@property (strong) NSString *duration;
@property (strong) NSString *ProjectColor;
@property (strong) NSString *ProjectGUID; // when project is not saved to backend yet
@property (strong) NSString *GUID;
@property BOOL billable;
@property BOOL unsynced;
@property (strong) NSDate *started;
@property (strong) NSDate *ended;
@property (strong) NSString *startTimeString;
@property (strong) NSString *endTimeString;
@property (strong) NSArray *tags;
@property (strong) NSDate *updatedAt;
@property (strong) NSString *formattedDate;
// If this is set to YES, a date header is displayed on top of
// the time entry cell in the time entry list view:
@property BOOL isHeader;
// If it's a header, this is useful:
@property (strong) NSString *dateDuration;
// If TE is passed for editing, this is the field that should be focussed
@property (strong) NSString *focusedFieldName;
@property BOOL CanSeeBillable;
@property uint64_t DefaultWID;
@property BOOL CanAddProjects;
@property (strong) NSString *WorkspaceName;
@property (strong) NSString *Error;
// if the item is load more row
@property BOOL loadMore;
// if item can be deleted without confirm
@property BOOL confirmlessDelete;
// Grouped Mode attributes
// If entry is group header
@property BOOL Group;
// Group Open state
@property BOOL GroupOpen;
// Group Name
@property (strong) NSString *GroupName;
// Group Duration
@property (strong) NSString *GroupDuration;
// Group Item Count
@property uint64_t GroupItemCount;
@end
