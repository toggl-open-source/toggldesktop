//
//  Settings.m
//  TogglDesktop
//
//  Created by Tanel Lebedev on 10/05/14.
//  Copyright (c) 2014 Toggl Desktop developers. All rights reserved.
//

#import "Settings.h"

@implementation Settings

- (void)load:(KopsikSettingsViewItem *)data
{
	self.timeline_recording_enabled = data->record_timeline;

	self.idle_detection = data->use_idle_detection;
	self.menubar_timer = data->menubar_timer;
	self.dock_icon = data->dock_icon;
	self.on_top = data->on_top;
	self.reminder = data->reminder;

	self.use_proxy = data->use_proxy;

	self.proxy_host = [NSString stringWithUTF8String:data->proxy_host];
	self.proxy_port = data->proxy_port;
	self.proxy_username = [NSString stringWithUTF8String:data->proxy_username];
	self.proxy_password = [NSString stringWithUTF8String:data->proxy_password];
}

@end
