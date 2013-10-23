//
//  TimeEntryListViewController.h
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "kopsik_api.h"

@interface TimeEntryListViewController : NSViewController
{
  @private
  NSDate *lastDate;
  NSMutableArray *viewitems;
  NSDateFormatter *dateFormat;
}
@property (weak) IBOutlet NSTableView *timeEntriesTableView;
- (IBAction)continueButtonClicked:(id)sender;
- (IBAction)performClick:(id)sender;
@end
