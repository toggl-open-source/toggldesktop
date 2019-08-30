//
//  UserNotificationCenter.h
//  TogglDesktop
//
//  Created by Nghia Tran on 12/4/18.
//  Copyright © 2018 Alari. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface UserNotificationCenter : NSObject

+ (instancetype)share;

- (void)scheduleReminderWithTitle:(NSString *)title informativeText:(NSString *)informativeText;

- (void)schedulePomodoroWithTitle:(NSString *)title informativeText:(NSString *)informativeText;

- (void)schedulePomodoroBreakWithTitle:(NSString *)title informativeText:(NSString *)informativeText;

- (void)scheduleAutoTrackerWithProjectName:(NSString *)projectName projectID:(NSNumber *)projectID taskID:(NSNumber *)taskID;

- (BOOL)isDoNotDisturbEnabled;

@end

NS_ASSUME_NONNULL_END
