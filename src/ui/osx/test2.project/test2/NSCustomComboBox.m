//
//  NSCustomComboBox.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 13/11/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "NSCustomComboBox.h"
#import "NSCustomComboBoxCell.h"

@implementation NSCustomComboBox
+ (void)load
{
    [self setCellClass:[NSCustomComboBoxCell class]];
}

- (void)reloadingData:(NSInteger)length {
	[super reloadData];
	[_cell setCalculatedMaxWidth:fmax((8*(int)length),_frame.size.width)];
}

@end
