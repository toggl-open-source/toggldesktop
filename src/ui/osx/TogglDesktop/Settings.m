//
//  Settings.m
//  TogglDesktop
//
//  Copyright (c) 2014 Toggl Desktop developers. All rights reserved.
//

#import "Settings.h"

@implementation Settings

- (void)load:(TogglSettingsView *)data
{
	self.timeline_recording_enabled = data->RecordTimeline;

	self.idle_detection = data->UseIdleDetection;
	self.menubar_timer = data->MenubarTimer;
	self.dock_icon = data->DockIcon;
	self.on_top = data->OnTop;
	self.reminder = data->Reminder;
	self.idle_minutes = (int)data->IdleMinutes;
	self.focus_on_shortcut = data->FocusOnShortcut;

	self.use_proxy = data->UseProxy;

	self.proxy_host = [NSString stringWithUTF8String:data->ProxyHost];
	self.proxy_port = data->ProxyPort;
	self.proxy_username = [NSString stringWithUTF8String:data->ProxyUsername];
	self.proxy_password = [NSString stringWithUTF8String:data->ProxyPassword];
}

@end
