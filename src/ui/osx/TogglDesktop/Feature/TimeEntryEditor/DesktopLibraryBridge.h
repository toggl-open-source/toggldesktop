//
//  DesktopLibraryBridge.h
//  TogglDesktop
//
//  Created by Nghia Tran on 4/2/19.
//  Copyright © 2019 Alari. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@class TimeEntryViewItem;
@class AutocompleteItem;

@interface DesktopLibraryBridge : NSObject

+ (instancetype)shared;

#pragma mark - Editor

- (NSString *)convertDuratonInSecond:(int64_t)durationInSecond;

- (NSString *)createClientWithWorkspaceID:(uint64_t)workspaceID
							   clientName:(NSString *)clientName;

- (NSString *)createProjectWithTimeEntryGUID:(NSString *)timeEntryGUID
								 workspaceID:(uint64_t)workspaceID
									clientID:(uint64_t)clientID
								  clientGUID:(NSString *_Nullable)clientGUID
								 projectName:(NSString *)projectName
									colorHex:(NSString *)colorHex
									isPublic:(BOOL)isPublic;

- (void)setBillableForTimeEntryWithTimeEntryGUID:(NSString *)timeEntryGUID
									  isBillable:(BOOL)isBillable;

- (void)setProjectForTimeEntryWithGUID:(NSString *)guid
								taskID:(uint64_t)taskID
							 projectID:(uint64_t)projectID
						   projectGUID:(NSString *)projectGUID;

- (void)updateTimeEntryWithDescription:(NSString *)descriptionName guid:(NSString *)guid;

- (void)updateTimeEntryWithTags:(NSArray<NSString *> *)tags guid:(NSString *)guid;

- (void)updateTimeEntryWithStartDate:(NSDate *)startDate
								guid:(NSString *)guid;

- (void)updateTimeEntryWithDuration:(NSString *)duration
							   guid:(NSString *)guid;

- (void)updateTimeEntryWithStartTime:(NSString *)startTime
								guid:(NSString *)guid;

- (void)updateTimeEntryWithStartAtTimestamp:(NSTimeInterval)timestamp
									   guid:(NSString *)guid
                            keepEndTimeFixed:(BOOL) keepEndTimeFixed;

- (void)updateTimeEntryWithEndTime:(NSString *)endTime
							  guid:(NSString *)guid;

- (void)updateTimeEntryWithEndAtTimestamp:(NSTimeInterval)timestamp
									 guid:(NSString *)guid;

- (void)deleteTimeEntryItem:(TimeEntryViewItem *)item undoManager:(NSUndoManager *_Nullable) undoManager;

- (void)updateDescriptionForTimeEntry:(TimeEntryViewItem *)timeEntry
						 autocomplete:(AutocompleteItem *)autocomplete;

#pragma mark - Timeline

- (void)enableTimelineRecord:(BOOL)isEnabled;

- (void)fetchTimelineData;

- (void)timelineSetPreviousDate;

- (void)timelineSetNextDate;

- (void)timelineSetDate:(NSDate *)date;

- (void)timelineGetCurrentDate;

- (NSString *_Nullable)starNewTimeEntryAtStarted:(NSTimeInterval)started ended:(NSTimeInterval)ended;

- (void)startEditorAtGUID:(NSString *)GUID;

- (void)closeEditor;

- (void)setEditorWindowSize:(CGSize)size;

- (CGSize)getEditorWindowSize;

#pragma mark - Editor

- (void)loadMoreTimeEntry;
- (void)setClickCloseBtnInAppMessage;
- (void)setClickActionBtnInAppMessage;
- (NSString * _Nullable)createNewTimeEntryWithOldTimeEntry:(TimeEntryViewItem *) item;

#pragma mark - Settings

- (void)setActiveTabAtIndex:(NSInteger) index;
- (NSInteger)getActiveTabIndex;

#pragma mark - Auto Tracker

- (void)enableAutoTracker:(BOOL)isEnabled;

#pragma mark - Formatter

- (NSString * _Nullable) formatDurationTimestampt:(NSTimeInterval) duration;

@end

NS_ASSUME_NONNULL_END
