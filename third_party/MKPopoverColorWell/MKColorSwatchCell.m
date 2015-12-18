//
//  MKColorSwatchCell.m
//  Color Picker
//
//  Created by Mark Dodwell on 5/27/12.
//  Copyright (c) 2012 mkdynamic. All rights reserved.
//

#import "MKColorSwatchCell.h"

@implementation MKColorSwatchCell
@synthesize color;

- (void)drawWithFrame:(NSRect)cellFrame
               inView:(NSMatrix *)controlView
{
    [super drawWithFrame:cellFrame inView:controlView];
    
    if ([color isEqualTo:[NSColor clearColor]]) {
        [[NSColor whiteColor] setFill];
        NSRectFill(cellFrame);
        
        NSBezierPath *line = [NSBezierPath bezierPath];
        [line moveToPoint:NSMakePoint(cellFrame.size.width, cellFrame.origin.y)];
        [line lineToPoint:NSMakePoint(cellFrame.origin.x, cellFrame.size.height)];
        [line setLineWidth:1.5];
        [[NSColor redColor] setStroke];
        
        [[NSGraphicsContext currentContext] saveGraphicsState];
        [[NSBezierPath bezierPathWithRect:cellFrame] setClip];
        [line stroke];
        [[NSGraphicsContext currentContext] restoreGraphicsState];
    } else {
        [color setFill];
        NSRectFill(cellFrame);
    }
}
@end
