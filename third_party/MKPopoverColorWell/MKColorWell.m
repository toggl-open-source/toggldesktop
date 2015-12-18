//
//  MKColorWell.m
//  Color Picker
//
//  Created by Mark Dodwell on 5/26/12.
//  Copyright (c) 2012 mkdynamic. All rights reserved.
//

#import "MKColorWell.h"
#import "MKColorPickerView.h"
#import "MKColorWell+Bindings.h"

@interface MKColorWell ()
- (void)setupPopover;
- (NSPopover *)createPopover;
- (NSViewController *)createPopoverViewController;
- (MKColorPickerView *)createPopoverView;
- (NSArray *)colorsForPopover;
@end

@implementation MKColorWell
@synthesize animatePopover = _animatePopover;

// NOTE you can subclass and define your own colors here
- (NSArray *)colorsForPopover
{
    // create popover view
    NSMutableArray *colors = [NSMutableArray array];
    NSColor *color;
    
    int stepHue = 12;
    int stepSaturation = 5;
    int stepBrightness = 5;
    int stepGray = stepSaturation + stepBrightness - 3;
    
    float hue;
    float saturation;
    float brightness;
    
    // transparent
    [colors addObject:[NSColor clearColor]];
    
    // gray
    hue = 0;
    saturation = 0;
    brightness = 1;
    while (brightness >= 0) {
        color = [NSColor colorWithDeviceHue:hue 
                                 saturation:saturation 
                                 brightness:brightness 
                                      alpha:1.0];
        [colors addObject:color];
        brightness -= 1.f / stepGray;
    }
    
    // colors
    hue = 0;
    while (hue <= 1.f) {
        saturation = 1.f / stepSaturation;
        brightness = 1;
        
        while (saturation <= 1.f) {
            color = [NSColor colorWithDeviceHue:hue 
                                     saturation:saturation 
                                     brightness:brightness 
                                          alpha:1.0];
            [colors addObject:color];
            saturation += 1.f / stepSaturation;
        }
        
        saturation = 1.f;
        brightness = 1 - (1.f / stepBrightness);
        while (brightness >= (1.f / stepBrightness)) {
            color = [NSColor colorWithDeviceHue:hue 
                                     saturation:saturation 
                                     brightness:brightness 
                                          alpha:1.0];
            [colors addObject:color];
            brightness -= 1.f / stepBrightness;
        }
        
        hue += 1.f / stepHue;
    }
    
    return colors;
}

- (NSPopover *)createPopover
{
    NSPopover *aPopover = [[NSPopover alloc] init];
    [aPopover setBehavior:NSPopoverBehaviorSemitransient];
    [aPopover setAnimates:self.animatePopover];
    return aPopover;
}

- (void)setAnimatePopover:(BOOL)animatePopover {
    _animatePopover = animatePopover;
    self->popover.animates = animatePopover;
}

- (NSViewController *)createPopoverViewController
{
    NSViewController *aPopoverViewController = [[NSViewController alloc] init];
    return aPopoverViewController;
}

- (MKColorPickerView *)createPopoverView
{
    NSArray *colors = [self colorsForPopover];
    uint rows = 13;
    NSSize swatchSize = NSMakeSize(15, 15);
    
    MKColorPickerView *aPopoverView;
    aPopoverView = [[MKColorPickerView alloc] initWithColors:colors
                                                numberOfRows:rows
                                             numberOfColumns:[colors count] / rows
                                                  swatchSize:swatchSize
                                             targetColorWell:self];
    return aPopoverView;
}

- (void)setupPopover
{
    popover = [self createPopover];
    popoverViewController = [self createPopoverViewController];
    popoverView = [self createPopoverView];
    
    popover.contentViewController = popoverViewController;
    popoverViewController.view = popoverView;
}

- (void)setColorAndClose:(NSColor *)color
{
    [self setColor:color];
    [self propagateValue:color forBinding:@"value"];
    [popover performClose:self];
}

- (void)awakeFromNib
{
    [self setupPopover];
}

- (BOOL)isOpaque
{
    return NO;
}

- (void)drawRect:(NSRect)dirtyRect
{
    [[NSColor clearColor] set];
    NSRectFillUsingOperation([self bounds], NSCompositeSourceOver);
    
    NSRect offsetBounds = NSOffsetRect(NSInsetRect([self bounds], 1, 1), 0, 1);
    NSRect colorBlockBounds = NSInsetRect(offsetBounds, 2, 2);
    
    if ([self isEnabled]) {
        if ([self.color isEqualTo:[NSColor clearColor]]) {
            [[NSColor whiteColor] setFill];
            NSRectFill(colorBlockBounds);
            
            NSBezierPath *line = [NSBezierPath bezierPath];
            [line moveToPoint:NSMakePoint(NSMinX(offsetBounds), NSMinY(offsetBounds))];
            [line lineToPoint:NSMakePoint(NSMaxX(offsetBounds), NSMaxY(offsetBounds))];
            [line setLineWidth:1.5];
            [line setLineCapStyle:NSRoundLineCapStyle];
            [[NSColor redColor] setStroke];
            [[NSGraphicsContext currentContext] saveGraphicsState];
            [[NSBezierPath bezierPathWithRect:offsetBounds] setClip];
            [line stroke];
            [[NSGraphicsContext currentContext] restoreGraphicsState];
        } else {
            [self.color setFill];
            NSRectFill(colorBlockBounds);
        }
    } else {
        [[NSColor colorWithDeviceWhite:.9 alpha:1] setFill];
        NSRectFill(colorBlockBounds);
    }
    
    NSBezierPath *outline = [NSBezierPath bezierPathWithRect:NSInsetRect(offsetBounds, 0.5, 0.5)];
    [outline setLineWidth:1.0];
    
    if ([self isEnabled]) {
        [[NSColor grayColor] setStroke];
    } else {
        [[NSColor colorWithDeviceWhite:.75 alpha:1] setStroke];
    }
    [outline stroke];
    
    
    NSBezierPath *highlight = [NSBezierPath bezierPath];
    [highlight moveToPoint:NSMakePoint(offsetBounds.origin.x, offsetBounds.origin.y - 0.5)];
    [highlight lineToPoint:NSMakePoint(offsetBounds.size.width + 1, offsetBounds.origin.y - 0.5)];
    [highlight setLineWidth:1.0];
    if ([self isEnabled]) {
        [[NSColor whiteColor] setStroke];
    } else {
        [[NSColor colorWithDeviceWhite:1.0 alpha:0.25] setStroke];
    }
    [highlight stroke];
}

- (void)mouseDown:(NSEvent *)theEvent
{
    if (![self isEnabled]) return;
    
    if ([popover isShown]) {
        [popover performClose:self];
    } else {
        [popover showRelativeToRect:[self frame] 
                             ofView:[self superview] 
                      preferredEdge:NSMinYEdge];
    }
}
@end
