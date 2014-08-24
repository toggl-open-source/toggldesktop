//
//  ConvertHexColor.m
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 05/11/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "ConvertHexColor.h"

@implementation ConvertHexColor

+ (NSColor *)hexCodeToNSColor:(NSString *)hexCode
{
	unsigned int colorCode = 0;

	if (hexCode.length > 1)
	{
		NSString *numbers = [hexCode substringWithRange:NSMakeRange(1, [hexCode length] - 1)];
		NSScanner *scanner = [NSScanner scannerWithString:numbers];
		[scanner scanHexInt:&colorCode];
	}
	return [NSColor
			colorWithDeviceRed:((colorCode >> 16) & 0xFF) / 255.0
						 green:((colorCode >> 8) & 0xFF) / 255.0
						  blue:((colorCode) & 0xFF) / 255.0 alpha:1.0];
}

@end
