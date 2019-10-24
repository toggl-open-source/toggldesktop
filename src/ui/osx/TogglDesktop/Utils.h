//
//  Utils.h
//  TogglDesktop
//
//  Created by Tanel Lebedev on 07/05/14.
//  Copyright (c) 2014 Toggl Desktop developers. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface ScriptResult : NSObject
@property (copy, nonatomic) NSString *text;
@property (assign, nonatomic) NSInteger err;
- (void)append:(NSString *)moreText;
@end

@interface Utils : NSObject
+ (NSInteger)boolToState:(BOOL)value;
+ (unsigned int)stateToBool:(NSInteger)state;
+ (void)disallowDuplicateInstances;
+ (NSString *)applicationSupportDirectory:(NSString *)environment;
+ (bool)isNumeric:(NSString *)checkText;
+ (void)addUnderlineToTextField:(NSTextField *)field;
+ (void)setUpdaterChannel:(NSString *)channel;
+ (ScriptResult *)runScript:(NSString *)script;
+ (BOOL)deleteTimeEntryWithConfirmationWithGUID:(NSString *)guid title:(NSString *)title;
@end

BOOL wasLaunchedAsLoginOrResumeItem(void);
BOOL wasLaunchedAsHiddenLoginItem(void);
