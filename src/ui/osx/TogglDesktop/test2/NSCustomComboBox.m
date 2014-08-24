//
//  NSCustomComboBox.m
//  Toggl Desktop on the Mac
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

- (void)reloadingData:(NSNumber *)length
{
	[super reloadData];
	int n = [length intValue];
	[_cell setCalculatedMaxWidth:fmax(8 * n, _frame.size.width)];
}

@end
