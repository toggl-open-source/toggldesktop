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
                                                   name:kUIEventUserLoggedIn
                                                 object:nil];
      [[NSNotificationCenter defaultCenter] addObserver:self
                                               selector:@selector(eventHandler:)
                                                   name:kUIEventTimerStopped
                                                 object:nil];
      [[NSNotificationCenter defaultCenter] addObserver:self
                                               selector:@selector(eventHandler:)
                                                   name:kUIEventDelete
                                                 object:nil];
      [[NSNotificationCenter defaultCenter] addObserver:self
                                               selector:@selector(eventHandler:)
                                                   name:kUIEventUpdate
                                                 object:nil];
    }
    return self;
}

-(void)eventHandler: (NSNotification *) notification
{
  if ([notification.name isEqualToString:kUIEventUserLoggedIn]) {
    char err[KOPSIK_ERR_LEN];
    KopsikTimeEntryViewItemList *list = kopsik_time_entry_view_item_list_init();
    if (KOPSIK_API_SUCCESS != kopsik_time_entry_view_items(ctx, err, KOPSIK_ERR_LEN, list)) {
      [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventError
                                                          object:[NSString stringWithUTF8String:err]];
      kopsik_time_entry_view_item_list_clear(list);
      return;
    }
    [viewitems removeAllObjects];
    for (int i = 0; i < list->Length; i++) {
      KopsikTimeEntryViewItem *item = list->ViewItems[i];
      TimeEntryViewItem *model = [[TimeEntryViewItem alloc] init];
      [model load:item];
      [viewitems addObject:model];
    }
    kopsik_time_entry_view_item_list_clear(list);
    [self.timeEntriesTableView reloadData];

  } else if ([notification.name isEqualToString:kUIEventTimerStopped]) {
    if (notification.object == nil) {
      return;
    }
    TimeEntryViewItem *item = notification.object;
    [viewitems insertObject:item atIndex:0];
    [self.timeEntriesTableView reloadData];
    
  } else if ([notification.name isEqualToString:kUIEventDelete]) {
    TimeEntryViewItem *deleted = notification.object;
    for (int i = 0; i < [viewitems count]; i++) {
      TimeEntryViewItem *item = [viewitems objectAtIndex:i];
      if ([deleted.GUID isEqualToString:item.GUID]) {
        [viewitems removeObject:item];
        [self.timeEntriesTableView reloadData];
        return;
      }
    }

  } else if ([notification.name isEqualToString:kUIEventUpdate]) {
    NSString *GUID = notification.object;
    TimeEntryViewItem *updated = [TimeEntryViewItem findByGUID:GUID];
    for (int i = 0; i < [viewitems count]; i++) {
      TimeEntryViewItem *item = [viewitems objectAtIndex:i];
      if ([GUID isEqualToString:item.GUID]) {
        if (updated != nil) {
          [viewitems replaceObjectAtIndex:i withObject:updated];
          [self.timeEntriesTableView reloadData];
        }
        return;
      }
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
    if (cellView == nil) {
      cellView = [[TableViewCell alloc] init];
      cellView.identifier = @"TimeEntryCell";
    }
    cellView.GUID = item.GUID;
    cellView.colorTextField.backgroundColor = [self hexCodeToNSColor:item.color];
    cellView.descriptionTextField.stringValue = item.Description;
    if (item.project) {
      cellView.projectTextField.stringValue = item.project;
    }
    cellView.durationTextField.stringValue = item.duration;
    return cellView;
}

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

void finishPushAfterContinue(kopsik_api_result result, char *err, unsigned int errlen) {
  if (KOPSIK_API_SUCCESS != result) {
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventError
                                                        object:[NSString stringWithUTF8String:err]];
    free(err);
  }
}

- (IBAction)continueButtonClicked:(id)sender {
  NSButton *btn = sender;
  TableViewCell *cell = (TableViewCell*)[btn superview];
  NSString *guid = cell.GUID;
  char err[KOPSIK_ERR_LEN];
  KopsikTimeEntryViewItem *item = kopsik_time_entry_view_item_init();
  if (KOPSIK_API_SUCCESS != kopsik_continue(ctx, err, KOPSIK_ERR_LEN, [guid UTF8String], item)) {
    kopsik_time_entry_view_item_clear(item);
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventError
                                                        object:[NSString stringWithUTF8String:err]];
    return;
  }

  TimeEntryViewItem *te = [[TimeEntryViewItem alloc] init];
  [te load:item];
  kopsik_time_entry_view_item_clear(item);

  [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventTimerRunning object:te];

  kopsik_push_async(ctx, finishPushAfterContinue);
}

- (IBAction)performClick:(id)sender {
  NSInteger row = [self.timeEntriesTableView clickedRow];
  TimeEntryViewItem *item = [viewitems objectAtIndex:row];
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventTimeEntrySelected
                                                      object:item.GUID];
}

@end
