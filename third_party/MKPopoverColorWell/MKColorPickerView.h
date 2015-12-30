//
//  MKColorPickerView.h
//  Color Picker
//
//  Created by Mark Dodwell on 5/27/12.
//  Copyright (c) 2012 mkdynamic. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface MKColorPickerView : NSView {
    NSMatrix *matrix;
}

@property NSMatrix *matrix;

- (id)initWithColors:(NSArray *)colors
        numberOfRows:(NSInteger)rows
     numberOfColumns:(NSInteger)columns
          swatchSize:(NSSize)size
     targetColorWell:(NSColorWell *)aTargetColorWell;
@end
