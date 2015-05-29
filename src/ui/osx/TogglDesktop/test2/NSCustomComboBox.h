//
//  NSCustomComboBox.h
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 13/11/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface NSCustomComboBox : NSComboBox
- (void)reloadingData:(NSNumber *)length;
- (BOOL)isExpanded;
@end
