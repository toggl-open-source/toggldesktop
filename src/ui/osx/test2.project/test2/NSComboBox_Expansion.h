//
//  NSComboBox_Expansion.h
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 04/02/2014.
//  Copyright (c) 2014 kopsik developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface NSComboBox (ExpansionAPI)
@property (getter=isExpanded) BOOL expanded;
@end
