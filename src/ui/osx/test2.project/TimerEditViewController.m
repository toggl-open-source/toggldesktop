//
//  TimerEditViewController.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import "TimerEditViewController.h"
#import "UIEvents.h"
#import "AutocompleteItem.h"
#import "Context.h"
#import "ErrorHandler.h"

@interface TimerEditViewController ()
@property NSMutableArray *autocompleteItems;
@end

@implementation TimerEditViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
      self.autocompleteItems = [[NSMutableArray alloc] init];
      [[NSNotificationCenter defaultCenter] addObserver:self
                                               selector:@selector(eventHandler:)
                                                   name:kUIStateUserLoggedIn
                                                 object:nil];
    }
    
    return self;
}

- (IBAction)startButtonClicked:(id)sender {
  NSString *description = [self.descriptionComboBox stringValue];
  [[NSNotificationCenter defaultCenter] postNotificationName:kUICommandNew
                                                      object:description];
  [self.descriptionComboBox setStringValue:@""];
}

- (NSString *)comboBox:(NSComboBox *)comboBox completedString:(NSString *)partialString
{
  for (NSString *text in self.autocompleteItems) {
    if ([[text commonPrefixWithString:partialString
                              options:NSCaseInsensitiveSearch] length] == [partialString length]) {
      return text;
    }
  }
  return @"";
}

- (void)eventHandler: (NSNotification *) notification
{
  if ([notification.name isEqualToString:kUIStateUserLoggedIn]) {
    [self.autocompleteItems removeAllObjects];
    KopsikAutocompleteItemList *list = kopsik_autocomplete_item_list_init();
    char err[KOPSIK_ERR_LEN];
    kopsik_api_result res = kopsik_autocomplete_items(ctx,
    	err, KOPSIK_ERR_LEN, list, 1, 1, 1);
    if (KOPSIK_API_SUCCESS != res) {
      handle_error(res, err);
      kopsik_autocomplete_item_list_clear(list);
      return;
    }
    for (int i = 0; i < list->Length; i++) {
      AutocompleteItem *item = [[AutocompleteItem alloc] init];
      [item load:list->ViewItems[i]];
//FIXME:      [self.autocompleteItems addObject:item];
    }
    kopsik_autocomplete_item_list_clear(list);
    if (self.descriptionComboBox.dataSource == nil) {
      self.descriptionComboBox.usesDataSource = YES;
      self.descriptionComboBox.dataSource = self;
    }
    [self.descriptionComboBox reloadData];
    return;
  }
}

-(NSInteger)numberOfItemsInComboBox:(NSComboBox *)aComboBox{
  return [self.autocompleteItems count];
}

-(id)comboBox:(NSComboBox *)aComboBox objectValueForItemAtIndex:(NSInteger)row{
  return [self.autocompleteItems objectAtIndex:row];
}

- (NSUInteger)comboBox:(NSComboBox *)aComboBox indexOfItemWithStringValue:(NSString *)aString
{
  return [self.autocompleteItems indexOfObject:aString];
}

@end
