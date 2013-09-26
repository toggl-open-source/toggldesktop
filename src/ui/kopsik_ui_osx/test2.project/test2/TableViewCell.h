//
//  TableViewCell.h
//  kopsik_ui_osx
//
//  Created by Tambet Masik on 9/26/13.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface TableViewCell : NSTableCellView
@property (nonatomic,strong) IBOutlet NSTextField *colorTextField;
@property (nonatomic,strong) IBOutlet NSTextField *descriptionTextField;
@property (nonatomic,strong) IBOutlet NSTextField *projectTextField;
@property (nonatomic,strong) IBOutlet NSTextField *durationTextField;
@property (nonatomic,strong) IBOutlet NSButton *continueButton;
@end

