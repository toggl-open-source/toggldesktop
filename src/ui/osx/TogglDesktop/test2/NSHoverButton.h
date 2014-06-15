//
//  NSHoverButton.h
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 21/02/2014.
//  Copyright (c) 2014 Toggl Desktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface NSHoverButton : NSButton
- (void)mouseEntered:(NSEvent *)theEvent;
- (void)mouseExited:(NSEvent *)theEvent;
@end
