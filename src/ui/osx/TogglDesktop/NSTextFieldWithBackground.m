//
//  NSTextFieldWithBackground.m
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 05/03/2014.
//  Copyright (c) 2014 Alari. All rights reserved.
//

#import "NSTextFieldWithBackground.h"

@implementation NSTextFieldWithBackground

- (id)initWithFrame:(NSRect)frame
{
	self = [super initWithFrame:frame];
	if (self)
	{
		// Initialization code here.
	}
	return self;
}

- (void)drawRect:(NSRect)dirtyRect
{
	[super drawRect:dirtyRect];
}

@end
