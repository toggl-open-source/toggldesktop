//
//  NSComboBox_Expansion.h
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 04/02/2014.
//  Copyright (c) 2014 TogglDesktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface NSComboBox (ExpansionAPI)
@property (getter = isExpanded) BOOL expanded;
@end
