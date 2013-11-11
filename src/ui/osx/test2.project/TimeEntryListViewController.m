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
#import "TableGroupCell.h"
#import "Context.h"
#import "UIEvents.h"
#import "ModelChange.h"
#import "ErrorHandler.h"
#import "DateHeader.h"

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

-(void) updateHeader:(NSDate *)date withFormattedDate:(NSString *)formattedDate
{
  NSLog(@"updateHeader date: %@", date);
  DateHeader *header = nil;
  // Find header from view items list.
  for (int i=0; i < viewitems.count; i++) {
    if ([viewitems[i] isKindOfClass:[DateHeader class]]) {
      DateHeader *h = viewitems[i];
      if ([h.actualDate isEqualToDate:date]) {
        NSAssert(header == nil, @"header with same date already found");
        header = h;
      }
    }
  }
  // If header not found, find insert point
  if (header == nil) {
    header = [[DateHeader alloc] init];
    header.formattedDate = formattedDate;
    header.actualDate = date;
    header.duration = [self durationForDate:formattedDate];
    [viewitems addObject:header];
  }
}

-(NSString *)durationForDate:(NSString *)dateHeader
{
  char err[KOPSIK_ERR_LEN];
  const int kMaxDurationLength = 100;
  char duration[kMaxDurationLength];
  kopsik_api_result res = kopsik_duration_for_date_header(ctx,
                                                          err, KOPSIK_ERR_LEN,
                                                          [dateHeader UTF8String],
                                                          duration, kMaxDurationLength);
  if (res != KOPSIK_API_SUCCESS) {
    handle_error(res, err);
    return nil;
  }
  return [NSString stringWithUTF8String:duration];
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

    @synchronized(viewitems) {
      // All time entries are sorted by start at this point.
      [viewitems removeAllObjects];
      NSString *date = nil;
      for (int i = 0; i < list->Length; i++) {
        KopsikTimeEntryViewItem *item = list->ViewItems[i];
        TimeEntryViewItem *model = [[TimeEntryViewItem alloc] init];
        [model load:item];
        // Add header if necessary
        if (date == nil || ![date isEqualToString:model.date]) {
          DateHeader *header = [[DateHeader alloc] init];
          header.actualDate = model.started;
          header.formattedDate = model.date;
          header.duration = [self durationForDate:model.date];
          [viewitems addObject:header];
        }
        date = model.date;
        [viewitems addObject:model];
      }
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
      @synchronized(viewitems) {
        TimeEntryViewItem *found = nil;
        for (int i = 0; i < [viewitems count]; i++) {
          TimeEntryViewItem *item = [viewitems objectAtIndex:i];
          if (! [item isKindOfClass:[TimeEntryViewItem class]]) {
            continue;
          }
          if ([change.GUID isEqualToString:item.GUID]) {
            found = item;
            break;
          }
        }
        if (found) {
          [viewitems removeObject:found];
          [self updateHeader:found.started withFormattedDate:found.date];
        }
      }
      [self.timeEntriesTableView reloadData];
      return;
    }

    // Handle update
    TimeEntryViewItem *updated = [TimeEntryViewItem findByGUID:change.GUID];

    BOOL found = NO;
    @synchronized(viewitems) {
      for (int i = 0; i < [viewitems count]; i++) {
        TimeEntryViewItem *item = [viewitems objectAtIndex:i];
        if (![item isKindOfClass:[TimeEntryViewItem class]]) {
          continue;
        }
        if (![change.GUID isEqualToString:item.GUID]) {
          continue;
        }
        [viewitems replaceObjectAtIndex:i withObject:updated];
        [self updateHeader:updated.started withFormattedDate:updated.date];
        found = YES;
        break;
      }
    }
    
    if (found) {
      [self.timeEntriesTableView reloadData];
      return;
    }

    // Since TE was not found in our list, it must be a new time entry.
    // Insert it to list, if it's not tracking.
    if (updated.duration_in_seconds < 0) {
      return;
    }
    @synchronized(viewitems) {
      [self updateHeader:updated.started withFormattedDate:updated.date];
      [viewitems addObject:updated];
    }
    [self.timeEntriesTableView reloadData];
  }
}

- (int)numberOfRowsInTableView:(NSTableView *)tv
{
  int result = 0;
  @synchronized(viewitems) {
    result = (int)[viewitems count];
  }
  return result;
}

- (NSView *)tableView:(NSTableView *)tableView
   viewForTableColumn:(NSTableColumn *)tableColumn
                  row:(NSInteger)row {
    id item = 0;
    @synchronized(viewitems) {
      item = [viewitems objectAtIndex:row];
    }
    NSAssert(item != nil, @"view item from viewitems array is nil");
    TableViewCell *cellView = [tableView makeViewWithIdentifier:@"TimeEntryCell" owner:self];
    if ([item isKindOfClass:[TimeEntryViewItem class]]){
      [cellView load:item];
      [cellView.continueButton setTarget:cellView];
      [cellView.continueButton setAction:@selector(continueTimeEntry:)];
      
      return cellView;
    }
    if ([item isKindOfClass:[DateHeader class]]) {
      TableGroupCell *groupCell = [tableView makeViewWithIdentifier:@"GroupCell" owner:self];
      [groupCell load:item];
      return groupCell;
    }
    
    NSAssert(false, @"Unknown view item class");
    return nil;
}

- (CGFloat)tableView:(NSTableView *)tableView
         heightOfRow:(NSInteger)row {
  NSObject *item = 0;
  @synchronized(viewitems) {
    item = viewitems[row];
  }
  if ([item isKindOfClass:[TimeEntryViewItem class]]) {
    return [tableView rowHeight];
  }
  return 22;
}

- (IBAction)performClick:(id)sender {
  NSInteger row = [self.timeEntriesTableView clickedRow];
  TimeEntryViewItem *item = 0;
  @synchronized(viewitems) {
    item = [viewitems objectAtIndex:row];
  }
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateTimeEntrySelected
                                                      object:item.GUID];
}

@end
