//
//  MKColorPickerView.m
//  Color Picker
//
//  Created by Mark Dodwell on 5/26/12.
//  Copyright (c) 2012 mkdynamic. All rights reserved.
//

#import "MKColorSwatchMatrix.h"
#import "MKColorSwatchCell.h"
#import "MKColorWell.h"

@implementation MKColorSwatchMatrix
- (id)initWithFrame:(NSRect)frameRect
       numberOfRows:(NSInteger)rowsHigh 
    numberOfColumns:(NSInteger)colsWide 
             colors:(NSArray *)theColors 
    targetColorWell:(MKColorWell *)aTargetColorWell
{
    colCount = (int)colsWide;
    colors = theColors;
    
    self = [super initWithFrame:frameRect
                           mode:NSTrackModeMatrix 
                      cellClass:[MKColorSwatchCell class] 
                   numberOfRows:rowsHigh 
                numberOfColumns:colsWide];
    
    if (self) {
        targetColorWell = aTargetColorWell;
    }
    
    return self;
}

- (NSCell *)makeCellAtRow:(NSInteger)row column:(NSInteger)column
{
    MKColorSwatchCell *cell = (MKColorSwatchCell *)[super makeCellAtRow:row column:column];

    int index = (int)(column + (row * colCount));
    
    if (index < [colors count]) {
        cell.color = [colors objectAtIndex:index];
    }
    
    return cell;
}

- (void)mouseDown:(NSEvent *)theEvent
{
    NSPoint pt = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    
    NSInteger row;
    NSInteger column;
    BOOL hit = [self getRow:&row column:&column forPoint:pt];
    
    if (hit) {
        MKColorSwatchCell *cell = [self cellAtRow:row column:column];
        [targetColorWell setColorAndClose:[cell color]];
    }
}
@end
