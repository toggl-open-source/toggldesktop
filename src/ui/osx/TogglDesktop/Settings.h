//
//  Settings.h
//  TogglDesktop
//
//  Copyright (c) 2014 Toggl Desktop developers. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "toggl_api.h"

@interface Settings : NSObject
- (void)load:(TogglSettingsView *)data;
@property BOOL idle_detection;
@property BOOL menubar_timer;
@property BOOL menubar_project;
@property BOOL dock_icon;
@property BOOL on_top;
@property BOOL reminder;
@property BOOL autodetect_proxy;
@property BOOL use_proxy;
@property NSString *proxy_host;
@property long proxy_port;
@property NSString *proxy_username;
@property NSString *proxy_password;
@property BOOL timeline_recording_enabled;
@property int idle_minutes;
@property BOOL focus_on_shortcut;
@property int reminder_minutes;
@property BOOL manual_mode;
@property NSString *remind_starts;
@property NSString *remind_ends;
@property BOOL remind_mon;
@property BOOL remind_tue;
@property BOOL remind_wed;
@property BOOL remind_thu;
@property BOOL remind_fri;
@property BOOL remind_sat;
@property BOOL remind_sun;
@property BOOL autotrack;
@property BOOL open_editor_on_shortcut;
@property BOOL render_timeline;
@property BOOL pomodoro;
@property BOOL pomodoro_break;
@property int pomodoro_minutes;
@property int pomodoro_break_minutes;
@property (assign, nonatomic) BOOL stopWhenShutdown;
@end
