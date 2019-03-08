//
//  AutocompleteItem.h
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 18/11/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "toggl_api.h"

@class ProjectSnapshot;

@interface AutocompleteItem : NSObject
- (instancetype)initWithSnapshot:(ProjectSnapshot *)snapshot;
+ (NSMutableArray *)loadAll:(TogglAutocompleteView *)first;
- (void)load:(TogglAutocompleteView *)data;
- (void)save:(TogglAutocompleteView *)data;
@property (copy, nonatomic) NSString *Text;
@property (copy, nonatomic) NSString *Description;
@property (copy, nonatomic) NSString *ProjectAndTaskLabel;
@property (copy, nonatomic) NSString *TaskLabel;
@property (copy, nonatomic) NSString *ProjectLabel;
@property (copy, nonatomic) NSString *ClientLabel;
@property (copy, nonatomic) NSString *ProjectColor;
@property (copy, nonatomic) NSString *WorkspaceName;
@property (assign, nonatomic) uint64_t ID;
@property (assign, nonatomic) uint64_t ProjectID;
@property (assign, nonatomic) uint64_t WorkspaceID;
@property (assign, nonatomic) uint64_t TaskID;
@property (assign, nonatomic) uint64_t Type;
@property (assign, nonatomic) bool Billable;
@property (strong, nonatomic) NSArray *tags;
@end
