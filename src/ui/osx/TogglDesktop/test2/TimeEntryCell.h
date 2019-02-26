//
//  TimeEntryCell.h
//  Toggl Desktop on the Mac
//
//  Created by Tambet Masik on 9/26/13.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "TimeEntryViewItem.h"
#import "NSHoverButton.h"

typedef NS_ENUM (NSUInteger, CellType)
{
	CellTypeNormal,
	CellTypeGroup,
	CellTypeSubItemInGroup,
};

@interface TimeEntryCell : NSCollectionViewItem
@property (nonatomic, weak) IBOutlet NSTextField *descriptionTextField;
@property (nonatomic, copy) NSString *GUID;
@property (copy) NSString *GroupName;
@property (assign, nonatomic) CellType cellType;
@property (assign, nonatomic) BOOL Group;
@property (assign, nonatomic) BOOL GroupOpen;

@property long long GroupItemCount;
@property BOOL confirmless_delete;

- (void)render:(TimeEntryViewItem *)view_item;
- (void)focusFieldName;
- (void)setFocused;
- (void)openEdit;
- (void)setupGroupMode;
- (void)showHorizontalLine:(BOOL)show;
- (void)applyMaskForBottomCorner;

@end

