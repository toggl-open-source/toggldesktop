//
//  Settings.h
//  TogglDesktop
//
//  Created by Tanel Lebedev on 10/05/14.
//  Copyright (c) 2014 Toggl Desktop developers. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "kopsik_api.h"

@interface Settings : NSObject
- (void)load:(KopsikSettingsViewItem *)data;
@property BOOL idle_detection;
@property BOOL menubar_timer;
@property BOOL dock_icon;
@property BOOL on_top;
@property BOOL reminder;
@property BOOL ignore_cert;
@property BOOL use_proxy;
@property NSString *proxy_host;
@property long proxy_port;
@property NSString *proxy_username;
@property NSString *proxy_password;
@property BOOL timeline_recording_enabled;
@end
