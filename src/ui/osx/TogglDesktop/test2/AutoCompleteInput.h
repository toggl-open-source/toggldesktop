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

@interface AutoCompleteInput : UndoTextField <NSTableViewDelegate, NSTableViewDataSource, NSTextFieldDelegate>
@property NSNib *nibAutoCompleteTableCell;
@property AutoCompleteTableContainer *autocompleteTableContainer;
@property AutoCompleteTable *autocompleteTableView;
@property int posY;
@property int lastItemCount;
@property int maxVisibleItems;
@property int itemHeight;
@property NSLayoutConstraint *heightConstraint;
@property NSLayoutConstraint *leftConstraint;
@property NSLayoutConstraint *rightConstraint;
@property NSLayoutConstraint *topConstraint;
@property BOOL constraintsActive;
@property NSButton *actionButton;
- (void)toggleTableView:(int)itemCount;
- (void)setPos:(int)posy;
- (void)hide;
- (void)resetTable;
- (void)showAutoComplete:(BOOL)show;
- (void)setButton:(NSButton *)button;
@end
