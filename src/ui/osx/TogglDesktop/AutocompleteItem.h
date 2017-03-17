//
//  AutocompleteItem.h
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 18/11/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "toggl_api.h"

@interface AutocompleteItem : NSObject
+ (NSMutableArray *)loadAll:(TogglAutocompleteView *)first;
- (void)load:(TogglAutocompleteView *)data;
- (void)save:(TogglAutocompleteView *)data;
@property NSString *Text;
@property NSString *Description;
@property NSString *ProjectAndTaskLabel;
@property NSString *TaskLabel;
@property NSString *ProjectLabel;
@property NSString *ClientLabel;
@property NSString *ProjectColor;
@property uint64_t ProjectID;
@property uint64_t WorkspaceID;
@property uint64_t TaskID;
@property uint64_t Type;
@property bool Billable;
@property (strong) NSArray *tags;
@end
