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

- (void)togglEditor;

- (void)updateTimeEntryWithDescription:(NSString *)descriptionName guid:(NSString *)guid;

- (void)updateTimeEntryWithTags:(NSArray<NSString *> *)tags guid:(NSString *)guid;

- (void)updateTimeEntryWithStartDate:(NSDate *)startDate
								guid:(NSString *)guid;

- (void)updateTimeEntryWithDuration:(NSString *)duration
							   guid:(NSString *)guid;

- (void)updateTimeEntryWithStartTime:(NSString *)startTime
								guid:(NSString *)guid;

- (void)updateTimeEntryWithEndTime:(NSString *)endTime
							  guid:(NSString *)guid;

- (void)deleteTimeEntryImte:(TimeEntryViewItem *)item;

- (void)updateDescriptionForTimeEntry:(TimeEntryViewItem *)timeEntry
						 autocomplete:(AutocompleteItem *)autocomplete;

- (void)setEditorWindowSize:(CGSize)size;
- (CGSize)getEditorWindowSize;

- (void)setSeenInAppMessageWithID:(NSString *) IDMessage;

@end

NS_ASSUME_NONNULL_END
