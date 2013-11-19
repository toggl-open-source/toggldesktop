//
//  AutocompleteDataSource.h
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 19/11/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "AutocompleteItem.h"

@interface AutocompleteDataSource : NSObject
@property NSMutableArray *autocompleteOrderedKeys;
@property NSMutableDictionary *autocompleteDictionary;
- (NSString *)completedString:(NSString *)partialString;
- (void)fetch:(BOOL)withTimeEntries withTasks:(BOOL)withTasks withProjects:(BOOL)withProjects;
- (AutocompleteItem *)get:(NSString *)key;
- (NSUInteger)count;
- (NSString *)keyAtIndex:(NSInteger)row;
- (NSUInteger)indexOfKey:(NSString *)key;
@end
