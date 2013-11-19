//
//  AutocompleteDataSource.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 19/11/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import "AutocompleteDataSource.h"
#import "kopsik_api.h"
#import "Context.h"
#import "ErrorHandler.h"

@implementation AutocompleteDataSource

- (id) init
{
  self = [super init];
  
  self.autocompleteOrderedKeys = [[NSMutableArray alloc] init];
  self.autocompleteDictionary = [[NSMutableDictionary alloc] init];
  
  return self;
}

- (NSString *)completedString:(NSString *)partialString {
  for (NSString *text in self.autocompleteOrderedKeys) {
    if ([[text commonPrefixWithString:partialString
                              options:NSCaseInsensitiveSearch] length] == [partialString length]) {
      return text;
    }
  }
  return @"";
}

- (NSString *)get:(NSString *)key
{
  return [self.autocompleteDictionary objectForKey:key];
}

- (void) fetch
{
  KopsikAutocompleteItemList *list = kopsik_autocomplete_item_list_init();
  char err[KOPSIK_ERR_LEN];
  kopsik_api_result res = kopsik_autocomplete_items(ctx, err, KOPSIK_ERR_LEN, list, 0, 0, 1);
  if (KOPSIK_API_SUCCESS != res) {
    kopsik_autocomplete_item_list_clear(list);
    handle_error(res, err);
    return;
  }
  
  // FIXME: synchronize this
  [self.autocompleteOrderedKeys removeAllObjects];
  [self.autocompleteDictionary removeAllObjects];
  for (int i = 0; i < list->Length; i++) {
    AutocompleteItem *item = [[AutocompleteItem alloc] init];
    [item load:list->ViewItems[i]];
    NSString *key = item.Text;
    if ([self.autocompleteDictionary valueForKey:key] == nil) {
      [self.autocompleteOrderedKeys addObject:key];
      [self.autocompleteDictionary setObject:item forKey:key];
    }
  }
  kopsik_autocomplete_item_list_clear(list);
}

- (NSUInteger)count {
  return [self.autocompleteOrderedKeys count];
}

- (NSString *)keyAtIndex:(NSInteger)row
{
  return [self.autocompleteOrderedKeys objectAtIndex:row];
}

- (NSUInteger)indexOfKey:(NSString *)key
{
  return [self.autocompleteOrderedKeys indexOfObject:key];
}

@end
