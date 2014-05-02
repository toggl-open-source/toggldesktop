//
//  TimeEntryListViewController.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "TimeEntryListViewController.h"
#import "TimeEntryViewItem.h"
#import "TimerEditViewController.h"
#import "UIEvents.h"
#import "kopsik_api.h"
#import "TimeEntryCell.h"
#import "TimeEntryCellWithHeader.h"
#import "Core.h"
#import "UIEvents.h"
#import "ModelChange.h"
#import "ErrorHandler.h"
#import "EditNotification.h"

@interface TimeEntryListViewController ()
@property NSTimer *timerTimeEntriesRendering;
@property (nonatomic, strong) IBOutlet TimerEditViewController *timerEditViewController;
@property NSNib *nibTimeEntryCell;
@property NSNib *nibTimeEntryCellWithHeader;
@end

@implementation TimeEntryListViewController

extern void *ctx;

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {

      self.timerEditViewController = [[TimerEditViewController alloc]
                                        initWithNibName:@"TimerEditViewController" bundle:nil];
      [self.timerEditViewController.view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];

      viewitems = [NSMutableArray array];

      self.nibTimeEntryCell = [[NSNib alloc] initWithNibNamed:@"TimeEntryCell"
                                                       bundle:nil];
      self.nibTimeEntryCellWithHeader = [[NSNib alloc] initWithNibNamed:@"TimeEntryCellWithHeader"
                                                                 bundle:nil];
      
      [[NSNotificationCenter defaultCenter] addObserver:self
                                               selector:@selector(eventHandler:)
                                                   name:kUIStateUserLoggedIn
                                                 object:nil];
      [[NSNotificationCenter defaultCenter] addObserver:self
                                               selector:@selector(eventHandler:)
                                                   name:kUIEventModelChange
                                                 object:nil];
      [[NSNotificationCenter defaultCenter] addObserver:self
                                               selector:@selector(eventHandler:)
                                                   name:kUIStateUserLoggedOut
                                                 object:nil];
      [[NSNotificationCenter defaultCenter] addObserver:self
                                               selector:@selector(eventHandler:)
                                                   name:kUIStateTimerRunning
                                                 object:nil];
      [[NSNotificationCenter defaultCenter] addObserver:self
                                               selector:@selector(eventHandler:)
                                                   name:kUIStateTimerStopped
                                                 object:nil];
    }
    return self;
}

-(void)loadView {
  [super loadView];
  [self.timeEntriesTableView registerNib:self.nibTimeEntryCell
                           forIdentifier:@"TimeEntryCell"];
  [self.timeEntriesTableView registerNib:self.nibTimeEntryCellWithHeader
                           forIdentifier:@"TimeEntryCellWithHeader"];

  [self.headerView addSubview:self.timerEditViewController.view];
  [self.timerEditViewController.view setFrame: self.headerView.bounds];
}

-(void)renderTimeEntries {
  NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

  NSLog(@"TimeEntryListViewController renderTimeEntries, thread %@", [NSThread currentThread]);  

  self.timerTimeEntriesRendering = nil;

  KopsikTimeEntryViewItem *first = 0;
  if (!kopsik_time_entry_view_items(ctx, &first)) {
    return;
  }

  @synchronized(viewitems) {
    // All time entries are sorted by start at this point.
    [viewitems removeAllObjects];
    NSString *date = nil;
    
    KopsikTimeEntryViewItem *item = first;
    while (item) {
      TimeEntryViewItem *model = [[TimeEntryViewItem alloc] init];
      [model load:item];
      if (date == nil || ![date isEqualToString:model.formattedDate]) {
        model.isHeader = YES;
      }
      date = model.formattedDate;
      [viewitems addObject:model];
      item = item->Next;
    }
  }
  
  kopsik_time_entry_view_item_clear(first);

  [self.timeEntriesTableView reloadData];
}

-(void)eventHandler: (NSNotification *) notification {
  if ([notification.name isEqualToString:kUIStateUserLoggedIn]) {
    [self performSelectorOnMainThread:@selector(scheduleRenderTimeEntries)
                           withObject:nil
                        waitUntilDone:NO];

    // Show header
    [self.headerView setHidden:NO];

    return;
  }

  if ([notification.name isEqualToString:kUIStateUserLoggedOut]) {
    [self.headerView setHidden:YES];
    return;
  }

  if ([notification.name isEqualToString:kUIEventModelChange]) {
    ModelChange *mc = notification.object;
    // On all TE changes, just re-render the list. It's Simpler.
    if ([mc.ModelType isEqualToString:@"tag"]) {
      return;
    }
    [self performSelectorOnMainThread:@selector(scheduleRenderTimeEntries)
                           withObject:nil
                        waitUntilDone:NO];
    return;
  }
}

- (void) scheduleRenderTimeEntries {
  NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

  if (self.timerTimeEntriesRendering != nil) {
    return;
  }
  @synchronized(self) {
    self.timerTimeEntriesRendering = [NSTimer scheduledTimerWithTimeInterval:kThrottleSeconds
                                                                       target:self
                                                                     selector:@selector(renderTimeEntries)
                                                                     userInfo:nil
                                                                      repeats:NO];
  }
}

- (int)numberOfRowsInTableView:(NSTableView *)tv {
  int result = 0;
  @synchronized(viewitems) {
    result = (int)[viewitems count];
  }
  return result;
}

- (NSView *)tableView:(NSTableView *)tableView
   viewForTableColumn:(NSTableColumn *)tableColumn
                  row:(NSInteger)row {
  TimeEntryViewItem *item = nil;
  @synchronized(viewitems) {
    item = [viewitems objectAtIndex:row];
  }
  NSAssert(item != nil, @"view item from viewitems array is nil");
  
  if (item.isHeader) {
    TimeEntryCellWithHeader *cell = [tableView makeViewWithIdentifier: @"TimeEntryCellWithHeader"
                                                      owner:self];
    [cell render:item];
    return cell;
  }

  TimeEntryCell *cell = [tableView makeViewWithIdentifier:@"TimeEntryCell"
                                                    owner:self];
  [cell render:item];
  return cell;
}

const int kDefaultRowHeight = 51;
const int kHeaderRowHeight = 102;

- (CGFloat)tableView:(NSTableView *)tableView
         heightOfRow:(NSInteger)row {
  TimeEntryViewItem *item = nil;
  @synchronized(viewitems) {
    if (row < viewitems.count) {
      item = viewitems[row];
    }
  }
  if (item && item.isHeader) {
    return kHeaderRowHeight;
  }
  return kDefaultRowHeight;
}

- (IBAction)performClick:(id)sender {
  NSInteger row = [self.timeEntriesTableView clickedRow];
  if (row < 0) {
    return;
  }
  TimeEntryViewItem *item = 0;
  @synchronized(viewitems) {
    item = viewitems[row];
  }
  EditNotification *edit = [[EditNotification alloc] init];
  edit.GUID = item.GUID;
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateTimeEntrySelected
                                                      object:edit];
}

- (void)tableViewSelectionDidChange:(NSNotification *)notification {
  NSInteger selectedRow = [self.timeEntriesTableView selectedRow];
  if (selectedRow < 0) {
    return;
  }
  NSTableRowView *rowView = [self.timeEntriesTableView rowViewAtRow:selectedRow
                                                    makeIfNecessary:NO];
  [rowView setEmphasized:NO];
  [rowView setSelected:NO];
}

@end
