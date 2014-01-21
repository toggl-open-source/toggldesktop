//
//  TimeEntryListViewController.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import "TimeEntryListViewController.h"
#import "TimeEntryViewItem.h"
#import "TimerViewController.h"
#import "TimerEditViewController.h"
#import "UIEvents.h"
#import "kopsik_api.h"
#import "TimeEntryCell.h"
#import "TimeEntryCellWithHeader.h"
#import "Context.h"
#import "UIEvents.h"
#import "ModelChange.h"
#import "ErrorHandler.h"

@interface TimeEntryListViewController ()
@property NSTimer *timerTimeEntriesRendering;
@property (nonatomic, strong) IBOutlet TimerViewController *timerViewController;
@property (nonatomic, strong) IBOutlet TimerEditViewController *timerEditViewController;
@property NSNib *nibTimeEntryCell;
@property NSNib *nibTimeEntryCellWithHeader;
@end

@implementation TimeEntryListViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {

      self.timerViewController = [[TimerViewController alloc]
                                  initWithNibName:@"TimerViewController" bundle:nil];
      [self.timerViewController.view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
      
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
}

-(void)renderTimeEntries {
  NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

  NSLog(@"TimeEntryListViewController renderTimeEntries, thread %@", [NSThread currentThread]);  

  self.timerTimeEntriesRendering = nil;

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
      if (date == nil || ![date isEqualToString:model.formattedDate]) {
        model.isHeader = YES;
      }
      date = model.formattedDate;
      [viewitems addObject:model];
    }
  }
  
  kopsik_time_entry_view_item_list_clear(list);

  [self.timeEntriesTableView reloadData];
}

-(void)eventHandler: (NSNotification *) notification
{
  if ([notification.name isEqualToString:kUIStateTimerStopped]) {
    // Hide running timer view from header view
    [self.timerViewController.view removeFromSuperview];
    
    // If timer editor is not visible yet, add it to header view
    for (int i = 0; i < [self.headerView subviews].count; i++) {
      if ([[self.headerView subviews] objectAtIndex:i] == self.timerEditViewController.view) {
        return;
      }
    }
    [self.headerView addSubview:self.timerEditViewController.view];
    [self.timerEditViewController.view setFrame:self.headerView.bounds];
    return;
  }

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
    [self.timerViewController.view removeFromSuperview];

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

  if ([notification.name isEqualToString:kUIStateTimerRunning]) {
    // Hide timer editor from header view
    [self.timerEditViewController.view removeFromSuperview];
    
    // If running timer view is not visible yet, add it to header view
    for (int i = 0; i < [self.headerView subviews].count; i++) {
      if ([[self.headerView subviews] objectAtIndex:i] == self.timerViewController.view) {
        return;
      }
    }
    [self.headerView addSubview:self.timerViewController.view];
    [self.timerViewController.view setFrame: self.headerView.bounds];
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
    [cell load:item];
    return cell;
  }

  TimeEntryCell *cell = [tableView makeViewWithIdentifier:@"TimeEntryCell"
                                                    owner:self];
  [cell load:item];
  return cell;
}

- (CGFloat)tableView:(NSTableView *)tableView
         heightOfRow:(NSInteger)row {
  TimeEntryViewItem *item = nil;
  @synchronized(viewitems) {
    item = viewitems[row];
  }
  if (item.isHeader) {
    return 76;
  }
  return 51;
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
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateTimeEntrySelected
                                                      object:item.GUID];
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
