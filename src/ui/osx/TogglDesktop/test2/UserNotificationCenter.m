//
//  UserNotificationCenter.m
//  TogglDesktop
//
//  Created by Nghia Tran on 12/4/18.
//  Copyright © 2018 Alari. All rights reserved.
//

#import "UserNotificationCenter.h"

@interface UserNotificationCenter()
@property (strong, nonatomic) NSUserNotificationCenter *center;
@end

@implementation UserNotificationCenter

-(instancetype) share {
    static UserNotificationCenter *instance;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[UserNotificationCenter alloc] init];
    });
    return instance;
}

-(instancetype)init {
    self = [super init];
    if (self) {
        self.center = [NSUserNotificationCenter defaultUserNotificationCenter];
    }
    return self;
}

-(NSUserNotification *) defaultUserNotificationWithTitle:(NSString *) title informativeText:(NSString *) informativeText {
    NSUserNotification *notification = [[NSUserNotification alloc] init];
    [notification setTitle:title];
    [notification setInformativeText:informativeText];

    // http://stackoverflow.com/questions/11676017/nsusernotification-not-showing-action-button
    [notification setValue:@YES forKey:@"_showsButtons"];
    [notification setDeliveryDate:[NSDate dateWithTimeInterval:0 sinceDate:[NSDate date]]];

    return notification;
}

-(void) deliveryNotification:(NSUserNotification *) notification {
    [self.center scheduleNotification:notification];
}

-(void) deliveryReminderWithTitle:(NSString *) title informativeText:(NSString *) informativeText {
    NSUserNotification *notification = [self defaultUserNotificationWithTitle:title
                                                              informativeText:informativeText];

    notification.userInfo = @{ @"reminder": @"YES" };
    notification.hasActionButton = YES;
    notification.actionButtonTitle = @"Track";
    notification.otherButtonTitle = @"Close";

    // Delivery
    [self deliveryNotification:notification];

    // Remove reminder after 45 seconds
    [self.center performSelector:@selector(removeDeliveredNotification:)
                 withObject:notification
                 afterDelay:45];
}

-(void) deliveryPomodoroWithTitle:(NSString *) title informativeText:(NSString *) informativeText {
    NSUserNotification *notification = [self defaultUserNotificationWithTitle:title
                                                              informativeText:informativeText];

    notification.userInfo = @{ @"pomodoro": @"YES" };
    notification.hasActionButton = YES;
    notification.actionButtonTitle = @"Continue";
    notification.otherButtonTitle = @"Close";

    // Delivery
    [self deliveryNotification:notification];

    // Play sound
    [[NSSound soundNamed:@"Glass"] play];
}

-(void) deliveryPomodoroBreakWithTitle:(NSString *) title informativeText:(NSString *) informativeText {
    NSUserNotification *notification = [self defaultUserNotificationWithTitle:title
                                                              informativeText:informativeText];

    notification.userInfo = @{ @"pomodoro_break": @"YES" };
    notification.hasActionButton = YES;
    notification.actionButtonTitle = @"Continue";
    notification.otherButtonTitle = @"Close";

    // Delivery
    [self deliveryNotification:notification];

    // Play sound
    [[NSSound soundNamed:@"Glass"] play];
}

-(void) deliveryAutoTrackerWithProjectName:(NSString *) projectName projectID:(NSNumber *) projectID taskID:(NSNumber *) taskID {
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
    [self deliveryNotification:notification];
}

@end
