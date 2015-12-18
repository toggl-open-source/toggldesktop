//
//  MKColorWell.h
//  Color Picker
//
//  Created by Mark Dodwell on 5/26/12.
//  Copyright (c) 2012 mkdynamic. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface MKColorWell : NSColorWell {
    NSPopover *popover;
    NSViewController *popoverViewController;
    NSView *popoverView;
}

@property (nonatomic, assign) BOOL animatePopover;

- (void)setColorAndClose:(NSColor *)aColor;

@end
