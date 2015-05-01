//
//  TimeEntryListViewController.h
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "toggl_api.h"
#import "NSUnstripedTableView.h"
#import "NSViewEscapable.h"
#import "NSTextFieldClickable.h"

@interface TimeEntryListViewController : NSViewController {
	@private
	NSMutableArray *viewitems;
}
@property (unsafe_unretained) IBOutlet NSView *headerView;
@property (unsafe_unretained) IBOutlet NSUnstripedTableView *timeEntriesTableView;
@property (strong) IBOutlet NSPopover *timeEntrypopover;
@property (strong) IBOutlet NSViewController *timeEntrypopoverViewController;
@property (strong) IBOutlet NSViewEscapable *timeEntryPopupEditView;
@property (strong) IBOutlet NSScrollView *timeEntryListScrollView;
@property (strong) IBOutlet NSTextFieldClickable *emptyLabel;
- (IBAction)performClick:(id)sender;
@end
