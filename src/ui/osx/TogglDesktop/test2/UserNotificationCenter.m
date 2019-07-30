//
//  UserNotificationCenter.m
//  TogglDesktop
//
//  Created by Nghia Tran on 12/4/18.
//  Copyright © 2018 Alari. All rights reserved.
//

#import "UserNotificationCenter.h"

@interface UserNotificationCenter ()
@property (strong, nonatomic) NSUserNotificationCenter *center;
@end

@implementation UserNotificationCenter

+ (instancetype)share
{
	static UserNotificationCenter *instance;
	static dispatch_once_t onceToken;

	dispatch_once(&onceToken, ^{
		instance = [[UserNotificationCenter alloc] init];
	});
	return instance;
}

- (instancetype)init
{
	self = [super init];
	if (self)
	{
		self.center = [NSUserNotificationCenter defaultUserNotificationCenter];
	}
	return self;
}

- (NSUserNotification *)defaultUserNotificationWithTitle:(NSString *)title informativeText:(NSString *)informativeText
{
	NSUserNotification *notification = [[NSUserNotification alloc] init];

	[notification setTitle:title];
	[notification setInformativeText:informativeText];

	// http://stackoverflow.com/questions/11676017/nsusernotification-not-showing-action-button
	[notification setValue:@YES forKey:@"_showsButtons"];
	[notification setDeliveryDate:[NSDate dateWithTimeInterval:0 sinceDate:[NSDate date]]];

	return notification;
}

- (void)scheduleNotification:(NSUserNotification *)notification
{
	[self.center scheduleNotification:notification];
}

- (void)removeAllDeliveredNotificationsWithType:(NSString *)type
{
	for (NSUserNotification *notification in self.center.deliveredNotifications)
	{
		if ([notification.userInfo objectForKey:type])
		{
			[self.center removeDeliveredNotification:notification];
		}
	}
}

- (void)scheduleReminderWithTitle:(NSString *)title informativeText:(NSString *)informativeText
{
	NSUserNotification *notification = [self defaultUserNotificationWithTitle:title
															  informativeText:informativeText];

	notification.userInfo = @{ @"reminder": @"YES" };
	notification.hasActionButton = YES;
	notification.actionButtonTitle = @"Track";
	notification.otherButtonTitle = @"Close";

	// Delivery
	[self removeAllDeliveredNotificationsWithType:@"reminder"];
	[self scheduleNotification:notification];

	// Remove reminder after 45 seconds
	[self.center performSelector:@selector(removeDeliveredNotification:)
					  withObject:notification
					  afterDelay:45];
}

- (void)schedulePomodoroWithTitle:(NSString *)title informativeText:(NSString *)informativeText
{
	NSUserNotification *notification = [self defaultUserNotificationWithTitle:title
															  informativeText:informativeText];

	notification.userInfo = @{ @"pomodoro": @"YES" };
	notification.hasActionButton = YES;
	notification.actionButtonTitle = @"Continue";
	notification.otherButtonTitle = @"Close";
	notification.soundName = @"Glass";

	// Delivery
	[self removeAllDeliveredNotificationsWithType:@"pomodoro"];
	[self scheduleNotification:notification];
}

- (void)schedulePomodoroBreakWithTitle:(NSString *)title informativeText:(NSString *)informativeText
{
	NSUserNotification *notification = [self defaultUserNotificationWithTitle:title
															  informativeText:informativeText];

	notification.userInfo = @{ @"pomodoro_break": @"YES" };
	notification.hasActionButton = YES;
	notification.actionButtonTitle = @"Continue";
	notification.otherButtonTitle = @"Close";
	notification.soundName = @"Glass";

	// Delivery
	[self removeAllDeliveredNotificationsWithType:@"pomodoro_break"];
	[self scheduleNotification:notification];
}

- (void)scheduleAutoTrackerWithProjectName:(NSString *)projectName projectID:(NSNumber *)projectID taskID:(NSNumber *)taskID
{
	NSUserNotification *notification = [self defaultUserNotificationWithTitle:@"Toggl Desktop Autotracker"
															  informativeText:[NSString stringWithFormat:@"Track %@?", projectName]];

	notification.hasActionButton = YES;
	notification.actionButtonTitle = @"Start";
	notification.otherButtonTitle = @"Close";
	notification.userInfo = @{
			@"autotracker": @"YES",
			@"project_id": projectID,
			@"task_id": taskID
	};

	// Delivery
	[self removeAllDeliveredNotificationsWithType:@"autotracker"];
	[self scheduleNotification:notification];
}

@end
