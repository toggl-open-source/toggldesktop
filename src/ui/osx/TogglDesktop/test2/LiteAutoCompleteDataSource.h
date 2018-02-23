//
//  LiteAutoCompleteDataSource.h
//  TogglDesktop
//
//  Created by Indrek Vändrik on 23/02/2018.
//  Copyright © 2018 Alari. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>
#import "AutoCompleteTable.h"
#import "AutocompleteItem.h"
#import "AutocompleteTableCell.h"

@interface LiteAutoCompleteDataSource : NSObject <NSTableViewDataSource>
@property NSMutableArray *orderedKeys;
@property NSMutableArray *filteredOrderedKeys;
@property NSMutableDictionary *dictionary;
@property NSString *currentFilter;
@property NSInteger textLength;
@property AutoCompleteTable *table;
- (NSString *)completedString:(NSString *)partialString;
- (AutocompleteItem *)get:(NSString *)key;
- (NSUInteger)count;
- (NSString *)keyAtIndex:(NSInteger)row;
- (NSUInteger)indexOfKey:(NSString *)key;
- (void)setFilter:(NSString *)filter;
- (id)initWithNotificationName:(NSString *)notificationName;
@end
