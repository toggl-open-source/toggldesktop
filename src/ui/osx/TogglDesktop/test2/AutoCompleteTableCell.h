//
//  AutoCompleteTableCell.h
//  LiteComplete
//
//  Created by Indrek Vändrik on 20/02/2018.
//  Copyright © 2018 Toggl. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "AutocompleteItem.h"
#import "ConvertHexColor.h"

@interface AutoCompleteTableCell : NSTableCellView
@property (weak) IBOutlet NSTextField *cellDescription;
@property (weak) IBOutlet NSBox *backgroundBox;
@property BOOL isSelectable;
- (void)render:(AutocompleteItem *)view_item selected:(BOOL)selected;
- (void)setFocused:(BOOL)focus;
- (NSMutableAttributedString *)setFormatedText:(AutocompleteItem *)view_item;
@end
