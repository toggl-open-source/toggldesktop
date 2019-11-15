//
//  AppIconFactory.m
//  TogglDesktop
//
//  Created by Nghia Tran on 11/19/18.
//  Copyright © 2018 Alari. All rights reserved.
//

#import "AppIconFactory.h"

@implementation AppIconFactory

+ (NSImage *)appIconWithType:(AppIconType)type
{
	NSImage *icon;

	switch (type)
	{
		case AppIconTypeActive :
			icon = [NSImage imageNamed:@"AppIconActive"];
			break;
		case AppIconTypeDefault :
			icon = [NSImage imageNamed:@"AppIcon"];
			break;
	}

	// Adapt well with black/white macOS appearance
	icon.template = YES;
	return icon;
}

@end
