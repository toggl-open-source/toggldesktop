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

-(instancetype) share;

-(void) deliveryReminderWithTitle:(NSString *) title informativeText:(NSString *) informativeText;

-(void) deliveryPomodoroWithTitle:(NSString *) title informativeText:(NSString *) informativeText;

-(void) deliveryPomodoroBreakWithTitle:(NSString *) title informativeText:(NSString *) informativeText;

-(void) deliveryAutoTrackerWithProjectName:(NSString *) projectName projectID:(NSString *) projectID taskID:(NSString *) taskID;

@end

NS_ASSUME_NONNULL_END
