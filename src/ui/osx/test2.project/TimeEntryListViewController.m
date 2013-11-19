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

-(void)renderTimeEntries
{
  NSLog(@"TimeEntryListViewController renderTimeEntries, thread %@", [NSThread currentThread]);
  
  NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");
  
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
        header.started = model.started;
        header.date = model.date;
        header.duration = model.dateDuration;
        [viewitems addObject:header];
      }
      date = model.date;
      [viewitems addObject:model];
    }
  }
  
  kopsik_time_entry_view_item_list_clear(list);

  [self.timeEntriesTableView reloadData];
  
  // 1) Headers with same date or formatted date are not allowed.
  NSHashTable *formattedDates = [[NSHashTable alloc] init];
  
  // 2) all view items must be ordered in descending order by date.
  NSDate *date = nil;
  
  // 3) two or more headers cannot follow each other
  DateHeader *previousHeader = nil;
  
  // Sanity checks, can/will remove when in production.
  for (int i = 0; i < viewitems.count; i++) {
    if ([viewitems[i] isKindOfClass:[TimeEntryViewItem class]]) {
      TimeEntryViewItem *item = viewitems[i];
      
      if (date != nil) {
        if ([item.started compare:date] == NSOrderedDescending) {
          [NSException raise:@"Invalid TE list rendering"
                      format:@"Previous date was %@, but now I've found %@ which is larger than date",
           date, item.started];
        }
      }
      
      date = item.started;
      previousHeader = nil;
      
    } else if ([viewitems[i] isKindOfClass:[DateHeader class]]) {
      if (previousHeader != nil) {
        [NSException raise:@"Header found to be empty"
                    format:@"Date headers should contain time entries, but header for %@ seems to be empty", previousHeader.started];
      }
      
      DateHeader *header = viewitems[i];
      
      if ([formattedDates containsObject:header.date]) {
        [NSException raise:@"Header already added with same date"
                    format:@"Header already added with same date: %@", date];
      }
      [formattedDates addObject:header.date];
      
      if (date != nil) {
        if([header.started compare:date] == NSOrderedDescending) {
          [NSException raise:@"Invalid header rendering"
                      format:@"Previous date was %@, but now I've found %@ which is larger than date",
           date, header.started];
          
        }
      }
      
      date = header.started;
      previousHeader = header;
    }
  }
}

-(void)eventHandler: (NSNotification *) notification
{
  // Handle log in: reload all time entries
  if ([notification.name isEqualToString:kUIStateUserLoggedIn]) {
    [self performSelectorOnMainThread:@selector(renderTimeEntries) withObject:nil waitUntilDone:NO];
  } else if ([notification.name isEqualToString:kUIEventModelChange]) {
    ModelChange *change = notification.object;
    // On all TE changes, just re-render the list. It's Simpler.
    if ([change.ModelType isEqualToString:@"time_entry"]) {
      [self performSelectorOnMainThread:@selector(renderTimeEntries) withObject:nil waitUntilDone:NO];
    }
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
  NSLog(@"Row clicked %ld", (long)row);
  id item = 0;
  @synchronized(viewitems) {
    item = viewitems[row];
  }
  if ([item isKindOfClass:[TimeEntryViewItem class]]) {
    TimeEntryViewItem *te = item;
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateTimeEntrySelected
                                                        object:te.GUID];
  }
}

@end
