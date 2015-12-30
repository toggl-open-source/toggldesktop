//
//  MKColorPickerView.m
//  Color Picker
//
//  Created by Mark Dodwell on 5/27/12.
//  Copyright (c) 2012 mkdynamic. All rights reserved.
//

#import "MKColorPickerView.h"
#import "MKColorSwatchMatrix.h"
#import "MKColorSwatchCell.h"
#import "MKColorWell.h"

@implementation MKColorPickerView
@synthesize matrix;

- (id)initWithColors:(NSArray *)colors 
        numberOfRows:(NSInteger)rows 
     numberOfColumns:(NSInteger)columns 
          swatchSize:(NSSize)size
     targetColorWell:(MKColorWell *)aTargetColorWell
{
    NSSize spacing = NSMakeSize(3, 3);
    NSRect matrixFrame = NSMakeRect(spacing.width, spacing.height, 
                                    columns * size.width + ((columns - 1) * spacing.width),
                                    rows * size.height + ((rows - 1)) * spacing.height);
    
    self = [self initWithFrame:NSInsetRect(matrixFrame, -spacing.width, -spacing.height)];
    
    if (self) {
        matrix = [[MKColorSwatchMatrix alloc] initWithFrame:matrixFrame 
                                               numberOfRows:rows 
                                            numberOfColumns:columns
                                                     colors:colors
                                            targetColorWell:aTargetColorWell];
        [matrix setIntercellSpacing:spacing];
        [matrix setCellSize:size];
        
        [self addSubview:matrix];
    } 
    
    return self;
}
@end
