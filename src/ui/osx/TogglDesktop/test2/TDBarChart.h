//
//  TDBarChart.h
//  TogglDesktop
//
//  Created by Indrek Vändrik on 03/03/16.
//  Copyright © 2016 Alari. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "ConvertHexColor.h"

@interface TDBarChart : NSView
- (void)setNewValue:(int)newValue;
@property long value;
@property NSColor *color;
@end
