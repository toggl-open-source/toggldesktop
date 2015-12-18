//
//  MKColorSwatchCell.h
//  Color Picker
//
//  Created by Mark Dodwell on 5/27/12.
//  Copyright (c) 2012 mkdynamic. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface MKColorSwatchCell : NSCell {
    NSColor *color;
}

@property (copy) NSColor *color;
@end
