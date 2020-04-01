//
//  TimeEntryCell.h
//  Toggl Desktop on the Mac
//
//  Created by Tambet Masik on 9/26/13.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class TimeEntryViewItem;

typedef NS_ENUM (NSUInteger, CellType)
{
	CellTypeNormal,
	CellTypeGroup,
	CellTypeSubItemInGroup,
};

@interface TimeEntryCell : NSCollectionViewItem
@property (nonatomic, strong, readonly) TimeEntryViewItem *item;
@property (nonatomic, copy, readonly) NSString *GUID;
@property (nonatomic, copy, readonly) NSString *GroupName;
@property (nonatomic, assign, readonly) BOOL GroupOpen;
@property (nonatomic, assign, readonly) CellType cellType;
@property (nonatomic, assign, readonly) BOOL confirmless_delete;
@property (nonatomic, copy, readonly) NSString *descriptionString;
@property (nonatomic, assign, readonly) BOOL Group;

- (void)render:(TimeEntryViewItem *)view_item;
- (void)focusFieldName;
- (void)setFocused;
- (void)openEdit;
- (void)setupGroupMode;
- (void)showHorizontalLine:(BOOL)show;
- (void)applyMaskForBottomCorner;

@end

