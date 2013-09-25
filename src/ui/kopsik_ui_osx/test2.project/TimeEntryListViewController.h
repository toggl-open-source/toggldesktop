//
//  TimeEntryListViewController.h
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 Alari. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "kopsik_api.h"

@interface TimeEntryListViewController : NSViewController
{
  @private
  NSMutableArray *viewitems;
}
@property (weak) IBOutlet NSTableView *timeEntriesTableView;
@end
