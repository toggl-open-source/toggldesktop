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
+ (NSArray<AutocompleteItem *> *)loadAll:(TogglAutocompleteView *)first;
- (void)load:(TogglAutocompleteView *)data;
- (void)save:(TogglAutocompleteView *)data;
@property NSString *Text;
@property NSString *Description;
@property (copy, nonatomic) NSString *DescriptionTitle; // Fix conflict with description in swift
@property NSString *ProjectAndTaskLabel;
@property NSString *TaskLabel;
@property NSString *ProjectLabel;
@property NSString *ClientLabel;
@property NSString *ProjectColor;
@property NSString *WorkspaceName;
@property uint64_t ID;
@property uint64_t ProjectID;
@property (copy, nonatomic) NSString *ProjectGUID;
@property uint64_t WorkspaceID;
@property uint64_t TaskID;
@property int64_t Type;
@property bool Billable;
@property (strong) NSArray *tags;
@end
