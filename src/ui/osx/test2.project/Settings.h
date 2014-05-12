//
//  Settings.h
//  TogglDesktop
//
//  Created by Tanel Lebedev on 10/05/14.
//  Copyright (c) 2014 Toggl Desktop developers. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface Settings : NSObject
@property BOOL idle_detection;
@property BOOL menubar_timer;
@property BOOL dock_icon;
@property BOOL on_top;
@property BOOL reminder;
@end
