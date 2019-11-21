//
//  TogglApplication.m
//  TogglDesktop
//
//  Created by Nghia Tran on 9/24/19.
//  Copyright © 2019 Alari. All rights reserved.
//

#import "TogglApplication.h"
#import <Bugsnag/Bugsnag.h>
#import "AppDelegate.h"

@implementation TogglApplication

- (void)reportException:(NSException *)exception
{
	AppDelegate *appDelegate = (AppDelegate *)[NSApp delegate];

	[Bugsnag notify:exception
			  block:^(BugsnagCrashReport *report) {
		 NSDictionary *data = @{
				 @"channel": [appDelegate currentChannel]
		 };
		 [report addMetadata:data toTabWithName:@"metadata"];
	 }];

	[super reportException:exception];
}

@end
