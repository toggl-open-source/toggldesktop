//
//  NSHoverButton.h
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 21/02/2014.
//  Copyright (c) 2014 Toggl Desktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface NSHoverButton : NSButton
@property CGFloat alpha;
- (void)setHoverAlpha:(CGFloat)alpha;
- (void)mouseEntered:(NSEvent *)theEvent;
- (void)mouseExited:(NSEvent *)theEvent;
@end
