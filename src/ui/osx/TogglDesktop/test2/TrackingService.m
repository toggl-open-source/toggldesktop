//
//  TrackingService.m
//  TogglDesktop
//
//  Created by Nghia Tran on 11/22/18.
//  Copyright © 2018 Alari. All rights reserved.
//

#import "TrackingService.h"

@implementation TrackingService

extern void *ctx;

+ (instancetype)sharedInstance
{
	static TrackingService *instance;
	static dispatch_once_t onceToken;

	dispatch_once(&onceToken, ^{
		instance = [[TrackingService alloc] init];
	});
	return instance;
}

- (void)trackWindowSize:(NSSize)size
{
	track_window_size(ctx,
					  @(size.width).integerValue,
					  @(size.height).integerValue);
}

@end
