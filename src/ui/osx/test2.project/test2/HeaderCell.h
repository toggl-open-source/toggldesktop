//
//  TableGroupCell.h
//  kopsik_ui_osx
//
//  Created by Tambet Masik on 10/24/13.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "DateHeader.h"

@interface HeaderCell : NSTableCellView
@property (nonatomic,strong) IBOutlet NSTextField *nameTextField;
- (void)load:(DateHeader *)header;
@end
