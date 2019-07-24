//
//  AutocompleteItem.h
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 18/11/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Foundation/Foundation.h>

@class ProjectSnapshot;

@interface AutocompleteItem : NSObject
@property (nonatomic, copy) NSString *Text;
@property (nonatomic, copy) NSString *Description;
@property (nonatomic, copy) NSString *DescriptionTitle; // Fix conflict with description in swift
@property (nonatomic, copy) NSString *ProjectAndTaskLabel;
@property (nonatomic, copy) NSString *TaskLabel;
@property (nonatomic, copy) NSString *ProjectLabel;
@property (nonatomic, copy) NSString *ClientLabel;
@property (nonatomic, copy) NSString *ProjectColor;
@property (nonatomic, copy) NSString *WorkspaceName;
@property (nonatomic, copy) NSString *ProjectGUID;
@property (nonatomic, assign) NSUInteger ID;
@property (nonatomic, assign) NSUInteger ProjectID;
@property (nonatomic, assign) NSUInteger WorkspaceID;
@property (nonatomic, assign) NSUInteger TaskID;
@property (nonatomic, assign) NSInteger Type;
@property (nonatomic, assign) BOOL Billable;
@property (nonatomic, strong) NSArray<NSString *> *tags;

- (instancetype)initWithSnapshot:(ProjectSnapshot *)snapshot;
+ (NSArray<AutocompleteItem *> *)loadAll:(TogglAutocompleteView *)first;
- (void)load:(TogglAutocompleteView *)data;
- (void)save:(TogglAutocompleteView *)data;
@end
