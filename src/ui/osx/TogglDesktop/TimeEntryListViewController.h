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

@protocol TimeEntryListViewControllerDelegate <NSObject>

- (BOOL)isTimerFocusing;
- (NSView *)containerViewForTimer;

@end

@interface TimeEntryListViewController : NSViewController
@property (weak, nonatomic) id<TimeEntryListViewControllerDelegate> delegate;
@property (strong) IBOutlet NSScrollView *timeEntryListScrollView;
@property (nonatomic, strong) TimeEntryDatasource *dataSource;
@property (assign, readonly, nonatomic) BOOL isEditorOpen;
@end
