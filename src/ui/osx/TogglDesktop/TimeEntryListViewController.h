//
//  TimeEntryListViewController.h
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class TimerEditViewController;
@class EditorPopover;

@protocol TimeEntryListViewControllerDelegate <NSObject>

- (BOOL)isTimerFocusing;
- (NSView *)containerViewForTimer;

@end

@interface TimeEntryListViewController : NSViewController
@property (weak, nonatomic) id<TimeEntryListViewControllerDelegate> delegate;
@property (nonatomic, strong) TimerEditViewController *timerEditViewController;
@property (nonatomic, strong) EditorPopover *timeEntrypopover;
@property (nonatomic, assign, readonly) BOOL isEditorOpen;

- (void)loadMoreIfNeedAtDate:(NSDate *)date;

@end
