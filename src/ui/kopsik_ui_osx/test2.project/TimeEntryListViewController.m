//
//  TimeEntryListViewController.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 Alari. All rights reserved.
//

#import "TimeEntryListViewController.h"
#import "TimeEntryViewItem.h"
#import "UIEvents.h"
#import "kopsik_api.h"
#import "Context.h"
#import "TableViewCell.h"

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

- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {
    TimeEntryViewItem *item = [viewitems objectAtIndex:row];
    TableViewCell *cellView = [tableView makeViewWithIdentifier:@"TimeEntryCell" owner:self];
    if (cellView == nil) {
      cellView = [[TableViewCell alloc] init];
      cellView.identifier = @"TimeEntryCell";
    }
    cellView.colorTextField.backgroundColor =
      [NSColor brownColor]; // FIXME: set project color
    cellView.descriptionTextField.stringValue = item.description;
    cellView.projectTextField.stringValue = item.project;
    cellView.durationTextField.stringValue = item.duration;
    NSLog(@"%d", cellView.durationTextField.isHidden);
    return cellView;
  /*
  // get an existing cell with the MyView identifier if it exists
  NSTextField *result = [tableView makeViewWithIdentifier:@"MyView" owner:self];
  
  // There is no existing cell to reuse so we will create a new one
  if (result == nil) {
    
    // create the new NSTextField with a frame of the {0,0} with the width of the table
    // note that the height of the frame is not really relevant, the row-height will modify the height
    // the new text field is then returned as an autoreleased object
    result = [[NSTextField alloc] init];
    
    // the identifier of the NSTextField instance is set to MyView. This
    // allows it to be re-used
    result.identifier = @"MyView";
  }
  
  // result is now guaranteed to be valid, either as a re-used cell
  // or as a new cell, so set the stringValue of the cell to the
  // nameArray value at row
  result.stringValue = @"blah";
  // return the result.
  return result;
   */
  
}

@end
