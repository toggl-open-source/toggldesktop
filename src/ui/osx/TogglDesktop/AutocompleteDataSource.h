//
//  AutocompleteDataSource.h
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 19/11/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "AutocompleteItem.h"

@interface AutocompleteDataSource : NSObject <NSComboBoxDataSource>
@property NSMutableArray *orderedKeys;
@property NSMutableArray *filteredOrderedKeys;
@property NSMutableDictionary *dictionary;
@property NSString *currentFilter;
@property NSInteger textLength;
@property NSComboBox *combobox;
- (NSString *)completedString:(NSString *)partialString;
- (AutocompleteItem *)get:(NSString *)key;
- (NSUInteger)count;
- (NSString *)keyAtIndex:(NSInteger)row;
- (NSUInteger)indexOfKey:(NSString *)key;
- (void)setFilter:(NSString *)filter;
- (id)initWithNotificationName:(NSString *)notificationName;
@end
