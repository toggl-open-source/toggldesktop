//
//  AutoCompleteTableCell.h
//  LiteComplete
//
//  Created by Indrek Vändrik on 20/02/2018.
//  Copyright © 2018 Toggl. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class AutocompleteItem;

@interface AutoCompleteTableCell : NSTableCellView
@property (nonatomic, assign, readonly) BOOL isSelectable;
- (void)render:(AutocompleteItem *)view_item selected:(BOOL)selected;
- (void)setFocused:(BOOL)focus;
@end
