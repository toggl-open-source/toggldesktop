//
//  Settings.h
//  TogglDesktop
//
//  Copyright (c) 2014 Toggl Desktop developers. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface Settings : NSObject
@property (nonatomic, assign) BOOL idle_detection;
@property (nonatomic, assign) BOOL menubar_timer;
@property (nonatomic, assign) BOOL menubar_project;
@property (nonatomic, assign) BOOL dock_icon;
@property (nonatomic, assign) BOOL on_top;
@property (nonatomic, assign) BOOL reminder;
@property (nonatomic, assign) BOOL autodetect_proxy;
@property (nonatomic, assign) BOOL use_proxy;
@property (nonatomic, copy) NSString *proxy_host;
@property (nonatomic, assign) NSInteger proxy_port;
@property (nonatomic, copy) NSString *proxy_username;
@property (nonatomic, copy) NSString *proxy_password;
@property (nonatomic, assign) BOOL timeline_recording_enabled;
@property (nonatomic, assign) NSInteger idle_minutes;
@property (nonatomic, assign) BOOL focus_on_shortcut;
@property (nonatomic, assign) NSInteger reminder_minutes;
@property (nonatomic, assign) BOOL manual_mode;
@property (nonatomic, copy) NSString *remind_starts;
@property (nonatomic, copy) NSString *remind_ends;
@property (nonatomic, assign) BOOL remind_mon;
@property (nonatomic, assign) BOOL remind_tue;
@property (nonatomic, assign) BOOL remind_wed;
@property (nonatomic, assign) BOOL remind_thu;
@property (nonatomic, assign) BOOL remind_fri;
@property (nonatomic, assign) BOOL remind_sat;
@property (nonatomic, assign) BOOL remind_sun;
@property (nonatomic, assign) BOOL autotrack;
@property (nonatomic, assign) BOOL open_editor_on_shortcut;
@property (nonatomic, assign) BOOL render_timeline;
@property (nonatomic, assign) BOOL pomodoro;
@property (nonatomic, assign) BOOL pomodoro_break;
@property (nonatomic, assign) NSInteger pomodoro_minutes;
@property (nonatomic, assign) NSInteger pomodoro_break_minutes;
@property (nonatomic, assign) BOOL stopWhenShutdown;

- (void)load:(TogglSettingsView *)data;
@end
