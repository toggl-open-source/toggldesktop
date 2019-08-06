//
//  LiteAutoCompleteDataSource.h
//  TogglDesktop
//
//  Created by Indrek Vändrik on 23/02/2018.
//  Copyright © 2018 Alari. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class AutocompleteItem;
@class AutoCompleteInput;

@interface LiteAutoCompleteDataSource : NSObject
@property (nonatomic, strong, readonly) NSMutableArray *filteredOrderedKeys;
@property (nonatomic, strong) AutoCompleteInput *input;
@property (nonatomic, copy) NSString *currentFilter;
- (NSString *)completedString:(NSString *)partialString;
- (void)clearFilter;
- (void)setFilter:(NSString *)filter;
- (id)initWithNotificationName:(NSString *)notificationName;
- (AutocompleteItem *)itemAtIndex:(NSInteger)row;
@end
