//
//  TableViewCell.h
//  kopsik_ui_osx
//
//  Created by Tambet Masik on 9/26/13.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "TimeEntryViewItem.h"

@interface TimeEntryCellWithHeader : NSTableCellView
@property (nonatomic, strong) IBOutlet NSTextField *descriptionTextField;
@property (nonatomic, strong) IBOutlet NSTextField *projectTextField;
@property (nonatomic, strong) IBOutlet NSTextField *durationTextField;
@property (nonatomic, strong) IBOutlet NSTextField *formattedDateTextField;
@property (nonatomic, strong) IBOutlet NSTextField *dateDurationTextField;
@property (strong) NSString *GUID;
- (void)render:(TimeEntryViewItem *)view_item;
- (IBAction)continueTimeEntry:(id)sender;
@end

