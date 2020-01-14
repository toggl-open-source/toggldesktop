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
            if (@available(macOS 10.13.0, *))
            {
                icon = [NSImage imageNamed:@"AppIcon"]; // NSImageNameApplicationIcon causes lost icon app in NSAlert in 10.13+
            } else {
                icon = [NSImage imageNamed:NSImageNameApplicationIcon];
            }

			break;
	}

	// Adapt well with black/white macOS appearance
	icon.template = YES;
	return icon;
}

@end
