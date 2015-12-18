//
//  MKColorPickerView.h
//  Color Picker
//
//  Created by Mark Dodwell on 5/26/12.
//  Copyright (c) 2012 mkdynamic. All rights reserved.
//

#import <Cocoa/Cocoa.h>
@class MKColorSwatchCell;
@class MKColorWell;

@interface MKColorSwatchMatrix : NSMatrix {
    NSArray *colors;
    MKColorWell *targetColorWell;
    int colCount;
}

- (id)initWithFrame:(NSRect)frameRect
       numberOfRows:(NSInteger)rowsHigh 
    numberOfColumns:(NSInteger)colsWide
             colors:(NSArray *)theColors
    targetColorWell:(MKColorWell *)aTargetColorWell;
@end
