//
//  TimeEntryListViewController.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import "TimeEntryListViewController.h"
#import "TimeEntryViewItem.h"
#import "UIEvents.h"
#import "kopsik_api.h"
#import "TableViewCell.h"
#import "Context.h"
#import "UIEvents.h"
#import "ModelChange.h"
#import "ErrorHandler.h"

@interface TimeEntryListViewController ()

@end

@implementation TimeEntryListViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
      viewitems = [NSMutableArray array];

      [[NSNotificationCenter defaultCenter] addObserver:self
                                               selector:@selector(eventHandler:)
                                                   name:kUIStateUserLoggedIn
                                                 object:nil];
      [[NSNotificationCenter defaultCenter] addObserver:self
                                               selector:@selector(eventHandler:)
                                                   name:kUIEventModelChange
                                                 object:nil];
    }
    return self;
}

-(void)eventHandler: (NSNotification *) notification
{
  if ([notification.name isEqualToString:kUIStateUserLoggedIn]) {
    char err[KOPSIK_ERR_LEN];
    KopsikTimeEntryViewItemList *list = kopsik_time_entry_view_item_list_init();
    if (KOPSIK_API_SUCCESS != kopsik_time_entry_view_items(ctx, err, KOPSIK_ERR_LEN, list)) {
      [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateError
                                                          object:[NSString stringWithUTF8String:err]];
      kopsik_time_entry_view_item_list_clear(list);
      return;
    }
    [viewitems removeAllObjects];
    for (int i = 0; i < list->Length; i++) {
      KopsikTimeEntryViewItem *item = list->ViewItems[i];
      TimeEntryViewItem *model = [[TimeEntryViewItem alloc] init];
      [model load:item];
      [viewitems addObject:@"Splitter"];
      [viewitems addObject:model];
    }
    kopsik_time_entry_view_item_list_clear(list);
    [self.timeEntriesTableView reloadData];
    
    return;
  }

  if ([notification.name isEqualToString:kUIEventModelChange]) {
    
    ModelChange *change = notification.object;
    
    // We only care about time entry changes
    if (! [change.ModelType isEqualToString:@"time_entry"]) {
      return;
    }
    
    // Handle delete
    if ([change.ChangeType isEqualToString:@"delete"]) {
      for (int i = 0; i < [viewitems count]; i++) {
        TimeEntryViewItem *item = [viewitems objectAtIndex:i];
        if (! [item isKindOfClass:[TimeEntryViewItem class]]) {
          continue;
        }
        if (! [change.GUID isEqualToString:item.GUID]) {
          continue;
        }
        [viewitems removeObject:item];
        [self.timeEntriesTableView reloadData];
        break;
      }
      
      return;
    }

    // Handle update
    TimeEntryViewItem *updated = [TimeEntryViewItem findByGUID:change.GUID];

    for (int i = 0; i < [viewitems count]; i++) {
      TimeEntryViewItem *item = [viewitems objectAtIndex:i];
      if (![item isKindOfClass:[TimeEntryViewItem class]]) {
        continue;
      }
      if (![change.GUID isEqualToString:item.GUID]) {
        continue;
      }
      [viewitems replaceObjectAtIndex:i withObject:updated];
      [self.timeEntriesTableView reloadData];
      break;
    }

    // Since TE was not found in our list, it must be a new time entry.
    // Insert it to list, if it's not tracking.
    if (updated.duration_in_seconds >= 0) {
      [viewitems insertObject:updated atIndex:0];
      [self.timeEntriesTableView reloadData];
    }
  }
}

- (int)numberOfRowsInTableView:(NSTableView *)tv
{
  return (int)[viewitems count];
}

- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {
    TimeEntryViewItem *item = [viewitems objectAtIndex:row];
    NSAssert(item != nil, @"view item from viewitems array is nil");
    TableViewCell *cellView = [tableView makeViewWithIdentifier:@"TimeEntryCell" owner:self];
    if ([item isKindOfClass:[TimeEntryViewItem class]]){
      cellView.GUID = item.GUID;
      cellView.colorTextField.backgroundColor = [self hexCodeToNSColor:item.color];
      cellView.descriptionTextField.stringValue = item.Description;
      if (item.project) {
        cellView.projectTextField.stringValue = item.project;
      }
      cellView.durationTextField.stringValue = item.duration;
      return cellView;
    } else {
      NSTableCellView *groupCell = [tableView makeViewWithIdentifier:@"GroupCell" owner:self];
      //[groupCell setStringValue:@"Today"];
      return groupCell;
    }
  return nil;
}

- (CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row {
  NSObject *item = viewitems[row];
  if ([item isKindOfClass:[TimeEntryViewItem class]]) {
    return [tableView rowHeight];
  }
  return 22;
}

/*
- (BOOL)tableView:(NSTableView *)tableView isGroupRow:(NSInteger)row {
  NSObject *item = viewitems[row];
  if ([item isKindOfClass:[TimeEntryViewItem class]]) {
    return NO;
  }
  return YES;
}
*/

- (NSColor *)hexCodeToNSColor:(NSString *)hexCode {
	unsigned int colorCode = 0;
  if (hexCode.length > 1) {
    NSString *numbers = [hexCode substringWithRange:NSMakeRange(1, [hexCode length] - 1)];
		NSScanner *scanner = [NSScanner scannerWithString:numbers];
		[scanner scanHexInt:&colorCode];
	}
	return [NSColor
          colorWithDeviceRed:((colorCode>>16)&0xFF)/255.0
          green:((colorCode>>8)&0xFF)/255.0
          blue:((colorCode)&0xFF)/255.0 alpha:1.0];
}

- (IBAction)continueButtonClicked:(id)sender {
  NSButton *btn = sender;
  TableViewCell *cell = (TableViewCell*)[btn superview];
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateError
                                                      object:cell.GUID];
}
- (IBAction)performClick:(id)sender {
  NSInteger row = [self.timeEntriesTableView clickedRow];
  TimeEntryViewItem *item = [viewitems objectAtIndex:row];
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateTimeEntrySelected
                                                      object:item.GUID];
}

@end
