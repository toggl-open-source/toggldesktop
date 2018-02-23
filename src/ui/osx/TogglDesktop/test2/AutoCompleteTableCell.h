//
//  AutoCompleteTableCell.h
//  LiteComplete
//
//  Created by Indrek Vändrik on 20/02/2018.
//  Copyright © 2018 Toggl. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "AutocompleteItem.h"

@interface AutoCompleteTableCell : NSTableCellView
@property (weak) IBOutlet NSTextField *cellDescription;
- (void)render:(AutocompleteItem *)view_item;
@end
