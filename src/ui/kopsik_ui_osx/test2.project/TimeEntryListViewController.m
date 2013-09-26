//
//  TimeEntryListViewController.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 Alari. All rights reserved.
//

#import "TimeEntryListViewController.h"
#import "timeEntryViewItem.h"
#import "UIEvents.h"
#import "kopsik_api.h"
#import "Context.h"

@interface TimeEntryListViewController ()

@end

@implementation TimeEntryListViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
      viewitems = [NSMutableArray array];

      [[NSNotificationCenter defaultCenter]
       addObserver:self
       selector:@selector(eventHandler:)
       name:kUIEventUserLoggedIn
       object:nil];
    }
    return self;
}

-(void)eventHandler: (NSNotification *) notification
{
  if ([notification.name isEqualToString:kUIEventUserLoggedIn]) {
    char err[KOPSIK_ERR_LEN];
    TogglTimeEntryViewItemList *list = kopsik_time_entry_view_item_list_init();
    if (KOPSIK_API_SUCCESS != kopsik_time_entry_view_items(ctx, err, KOPSIK_ERR_LEN, list)) {
      NSLog(@"Error fetching time entries: %s", err);
    } else {
      [viewitems removeAllObjects];
      for (int i = 0; i < list->Length; i++) {
        TogglTimeEntryViewItem *item = list->ViewItems[i];
        TimeEntryViewItem *model = [[TimeEntryViewItem alloc] init];
        model.description = [NSString stringWithUTF8String:item->Description];
        if (item->Project) {
          model.project = [NSString stringWithUTF8String:item->Project];
        }
        model.duration = [NSString stringWithUTF8String:item->Duration];
        [viewitems addObject:model];
      }
    }
    kopsik_time_entry_view_item_list_clear(list);
    [self.timeEntriesTableView reloadData];
  }
}

- (int)numberOfRowsInTableView:(NSTableView *)tv
{
  return (int)[viewitems count];
}

- (NSView *)tableView:(NSTableView *)tableView
   viewForTableColumn:(NSTableColumn *)tableColumn
                  row:(NSInteger)row {
  
  NSView *result = [tableView makeViewWithIdentifier:@"TimeEntryViewCell" owner:self];
  //TimeEntryViewItem *item = [viewitems objectAtIndex:row];
  
  return result;
}

@end
