//
//  AutoComleteInput.h
//  LiteComplete
//
//  Created by Indrek Vändrik on 20/02/2018.
//  Copyright © 2018 Toggl. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#include <Carbon/Carbon.h>
#import "AutoCompleteTable.h"
#import "AutoCompleteTableCell.h"
#import "AutoCompleteTableContainer.h"
#import "UndoTextField.h"

typedef NS_ENUM (NSUInteger, AutoCompleteDisplayMode)
{
	AutoCompleteDisplayModeCompact,
	AutoCompleteDisplayModeFullscreen,
};

@interface AutoCompleteInput : UndoTextField <NSTableViewDelegate, NSTableViewDataSource, NSTextFieldDelegate>
@property NSNib *nibAutoCompleteTableCell;
@property AutoCompleteTableContainer *autocompleteTableContainer;
@property AutoCompleteTable *autocompleteTableView;
@property int posY;
@property NSButton *actionButton;
@property (assign, nonatomic) AutoCompleteDisplayMode displayMode;
@property (assign, nonatomic) CGFloat itemHeight;
@property (assign, nonatomic) CGFloat worksapceItemHeight;
@property (assign, nonatomic) CGFloat totalHeight;

- (void)toggleTableViewWithNumberOfItem:(NSInteger)numberOfItem;
- (void)updateDropdownWithHeight:(CGFloat)height;
- (void)setPos:(int)posy;
- (void)hide;
- (void)resetTable;
- (void)showAutoComplete:(BOOL)show;
- (void)setButton:(NSButton *)button;
- (void)reloadAutocomplete:(NSArray<AutocompleteItem *> *)array;
- (CGFloat)calculateTotalHeightFromArray:(NSArray<AutocompleteItem *> *)array;
@end
