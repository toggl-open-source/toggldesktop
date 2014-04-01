//
//  AutocompleteDataSource.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 19/11/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "AutocompleteDataSource.h"
#import "kopsik_api.h"
#import "Context.h"
#import "ErrorHandler.h"

@implementation AutocompleteDataSource

- (id) init {
  self = [super init];

  self.orderedKeys = [[NSMutableArray alloc] init];
  self.filteredOrderedKeys = [[NSMutableArray alloc] init];
  self.dictionary = [[NSMutableDictionary alloc] init];
  
  return self;
}

- (NSString *)completedString:(NSString *)partialString {
  @synchronized(self) {
    for (NSString *text in self.filteredOrderedKeys) {
      if ([[text commonPrefixWithString:partialString
                                options:NSCaseInsensitiveSearch] length] == [partialString length]) {
        return text;
      }
    }
  }
  return @"";
}

- (NSString *)get:(NSString *)key {
  NSString *object = nil;
  @synchronized(self) {
    object = [self.dictionary objectForKey:key];
  }
  return object;
}

- (void) fetch:(BOOL)withTimeEntries
     withTasks:(BOOL)withTasks
  withProjects:(BOOL)withProjects {
  KopsikAutocompleteItem *first = 0;
  if (!kopsik_autocomplete_items(ctx,
                                 &first,
                                 withTimeEntries,
                                 withTasks,
                                 withProjects)) {
    return;
  }
  
  @synchronized(self) {
    [self.orderedKeys removeAllObjects];
    [self.dictionary removeAllObjects];
    KopsikAutocompleteItem *record = first;
    while (record) {
      AutocompleteItem *item = [[AutocompleteItem alloc] init];
      [item load:record];
      NSString *key = item.Text;
      if ([self.dictionary objectForKey:key] == nil) {
        [self.orderedKeys addObject:key];
        [self.dictionary setObject:item forKey:key];
      }
      record = record->Next;
    }
  }
  [self setFilter:self.currentFilter];
  kopsik_autocomplete_item_clear(first);
}

- (NSUInteger)count {
  NSUInteger result = 0;
  @synchronized(self) {
    result = [self.filteredOrderedKeys count];
  }
  return result;
}

- (NSString *)keyAtIndex:(NSInteger)row {
  NSString *key = nil;
  @synchronized(self) {
    key = [self.filteredOrderedKeys objectAtIndex:row];
  }
  return key;
}

- (NSUInteger)indexOfKey:(NSString *)key {
  NSUInteger index = 0;
  @synchronized(self) {
    return [self.filteredOrderedKeys indexOfObject:key];
  }
  return index;
}

- (void)setFilter:(NSString *)filter {
  self.textLength = 0;
  @synchronized(self) {
    self.currentFilter = filter;
    if (filter == nil || filter.length == 0) {
      self.filteredOrderedKeys = [NSMutableArray arrayWithArray:self.orderedKeys];
    } else {
      NSMutableArray *filtered = [[NSMutableArray alloc] init];
      for (int i = 0; i < self.orderedKeys.count; i++) {
        NSString *key = self.orderedKeys[i];
        if ([key rangeOfString:filter options:NSCaseInsensitiveSearch].location != NSNotFound) {
          if ([key length] > self.textLength){
            self.textLength = [key length];
          }
          [filtered addObject:key];
        }
      }
      self.filteredOrderedKeys = filtered;
    }
  }
}

@end
