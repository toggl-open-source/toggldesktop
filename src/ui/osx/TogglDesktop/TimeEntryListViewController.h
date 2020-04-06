//
//  TimeEntryListViewController.h
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@protocol TimeEntryListViewControllerDelegate <NSObject>

- (BOOL)isTimerFocusing;
- (NSView *)containerViewForTimer;
- (void) shouldFocusTimer;

@end

@interface TimeEntryListViewController : NSViewController
@property (weak, nonatomic) id<TimeEntryListViewControllerDelegate> delegate;
@property (nonatomic, assign, readonly) BOOL isEditorOpen;

- (void)loadMoreIfNeedAtDate:(NSDate *)date;
- (NSView * __nullable) firstTimeEntryCellForOnboarding;

@end
