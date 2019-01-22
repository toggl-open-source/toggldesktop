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
	self.menubar_project = data->MenubarProject;
	self.dock_icon = data->DockIcon;
	self.on_top = data->OnTop;
	self.reminder = data->Reminder;
	self.idle_minutes = (int)data->IdleMinutes;
	self.focus_on_shortcut = data->FocusOnShortcut;
	self.reminder_minutes = (int)data->ReminderMinutes;
	self.pomodoro = data->Pomodoro;
	self.pomodoro_minutes = (int)data->PomodoroMinutes;
	self.pomodoro_break = data->PomodoroBreak;
	self.pomodoro_break_minutes = (int)data->PomodoroBreakMinutes;
	self.manual_mode = data->ManualMode;

	self.use_proxy = data->UseProxy;

	self.proxy_host = [NSString stringWithUTF8String:data->ProxyHost];
	self.proxy_port = data->ProxyPort;
	self.proxy_username = [NSString stringWithUTF8String:data->ProxyUsername];
	self.proxy_password = [NSString stringWithUTF8String:data->ProxyPassword];

	self.autodetect_proxy = data->AutodetectProxy;

	self.remind_starts = [NSString stringWithUTF8String:data->RemindStarts];
	self.remind_ends = [NSString stringWithUTF8String:data->RemindEnds];

	self.remind_mon = data->RemindMon;
	self.remind_tue = data->RemindTue;
	self.remind_wed = data->RemindWed;
	self.remind_thu = data->RemindThu;
	self.remind_fri = data->RemindFri;
	self.remind_sat = data->RemindSat;
	self.remind_sun = data->RemindSun;

	self.autotrack = data->Autotrack;

	self.open_editor_on_shortcut = data->OpenEditorOnShortcut;
	self.stopWhenShutdown = data->StopEntryOnShutdownSleep;
}

@end
