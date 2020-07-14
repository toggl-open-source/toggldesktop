//
//  NSComboBox_Expansion.h
//  Toggl Track on the Mac
//
//  Created by Tanel Lebedev on 04/02/2014.
//  Copyright (c) 2014 TogglTrack developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface NSComboBox (ExpansionAPI)
@property (nonatomic, assign, getter = isExpanded) BOOL expanded;
@end
