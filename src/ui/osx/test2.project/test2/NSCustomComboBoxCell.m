//
//  NSCustomComboBoxCell.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 13/11/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "NSCustomComboBoxCell.h"

@implementation NSCustomComboBoxCell
- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
    NSRect myRect = NSMakeRect(cellFrame.origin.x,cellFrame.origin.y,
                               500,cellFrame.size.height);
    [super drawWithFrame:myRect inView:controlView];
}
@end
