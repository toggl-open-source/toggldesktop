//
//  TimeEntryListViewController.h
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "kopsik_api.h"
#import "NSUnstripedTableView.h"

@interface TimeEntryListViewController : NSViewController {
  @private
  NSMutableArray *viewitems;
}
@property (weak) IBOutlet NSView *headerView;
@property (weak) IBOutlet NSUnstripedTableView *timeEntriesTableView;
- (IBAction)performClick:(id)sender;
@end
