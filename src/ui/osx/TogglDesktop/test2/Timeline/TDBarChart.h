//
//  TDBarChart.h
//  TogglDesktop
//
//  Created by Indrek Vändrik on 27/03/2019.
//  Copyright © 2019 Alari. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "ConvertHexColor.h"

NS_ASSUME_NONNULL_BEGIN

@interface TDBarChart : NSView
- (void)setNewValue:(int)newValue;
@property long value;
@property NSColor *color;
@end

NS_ASSUME_NONNULL_END
