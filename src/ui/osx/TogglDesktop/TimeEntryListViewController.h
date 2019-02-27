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

@class EditorPopover;
@class TimeEntryDatasource;

@interface TimeEntryListViewController : NSViewController
@property (unsafe_unretained) IBOutlet NSView *headerView;
@property (strong) IBOutlet EditorPopover *timeEntrypopover;
@property (strong) IBOutlet NSViewController *timeEntrypopoverViewController;
@property (strong) IBOutlet NSViewEscapable *timeEntryPopupEditView;
@property (strong) IBOutlet NSScrollView *timeEntryListScrollView;
@property (strong) IBOutlet NSTextFieldClickable *emptyLabel;
@property (nonatomic, strong) TimeEntryDatasource *dataSource;
@end
