//
//  NSTextField+Ext.m
//  TogglDesktop
//
//  Created by Nghia Tran on 11/20/18.
//  Copyright © 2018 Alari. All rights reserved.
//

#import "NSTextField+Ext.h"

@implementation NSTextField (Extension)

- (void)resetCursorColor
{
	NSTextView *editor = (NSTextView *)[self currentEditor];

	if ([editor respondsToSelector:@selector(setInsertionPointColor:)])
	{
		[editor setInsertionPointColor:[self textColor]];
	}
}

@end
