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

@interface AutoCompleteInput : NSTextField <NSTableViewDelegate, NSTableViewDataSource, NSTextFieldDelegate>
@property NSNib *nibAutoCompleteTableCell;
@property AutoCompleteTableContainer *autocompleteTableContainer;
@property AutoCompleteTable *autocompleteTableView;
@property int posY;
@property int lastItemCount;
@property int maxVisibleItems;
@property int itemHeight;
@property NSLayoutConstraint *heightConstraint;
- (void)toggleTableView:(int)itemCount;
- (void)setPos:(int)posy;
- (void)hide;
- (void)resetTable;
- (void)showAutoComplete:(BOOL)show;
@end
