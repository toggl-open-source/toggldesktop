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

NS_ASSUME_NONNULL_BEGIN
@interface LiteAutoCompleteDataSource : NSObject
@property (nonatomic, strong, readonly) NSMutableArray *filteredOrderedKeys;
@property (nonatomic, strong, nullable) AutoCompleteInput *input;
@property (nonatomic, copy, nullable) NSString *currentFilter;

- (id)initWithNotificationName:(NSString *)notificationName;
- (NSString *)completedString:(NSString *)partialString;
- (void)clearFilter;
- (void)setFilter:(nullable NSString *)filter;
- (nullable AutocompleteItem *)itemAtIndex:(NSInteger)row;
@end
NS_ASSUME_NONNULL_END
