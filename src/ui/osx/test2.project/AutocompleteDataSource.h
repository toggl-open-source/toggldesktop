//
//  AutocompleteDataSource.h
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 19/11/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "AutocompleteItem.h"

@interface AutocompleteDataSource : NSObject
@property NSMutableArray *orderedKeys;
@property NSMutableArray *filteredOrderedKeys;
@property NSMutableDictionary *dictionary;
@property NSString *currentFilter;
@property NSInteger textLength;
- (NSString *)completedString:(NSString *)partialString;
- (void)fetch:(BOOL)withTimeEntries withTasks:(BOOL)withTasks withProjects:(BOOL)withProjects;
- (AutocompleteItem *)get:(NSString *)key;
- (NSUInteger)count;
- (NSString *)keyAtIndex:(NSInteger)row;
- (NSUInteger)indexOfKey:(NSString *)key;
- (void)setFilter:(NSString *)filter;
@end
