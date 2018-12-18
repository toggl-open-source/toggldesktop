//
//  NSAlert+Utils.m
//  TogglDesktop
//
//  Created by Nghia Tran on 12/12/18.
//  Copyright © 2018 Alari. All rights reserved.
//

#import "NSAlert+Utils.h"

@implementation NSAlert (Utils)

+ (NSAlert *)alertWithMessageText:(NSString *)message informativeTextWithFormat:(NSString *)format, ...
{
	NSAlert *alert = [[NSAlert alloc] init];

	alert.messageText = message;
	alert.informativeText = format;
	return alert;
}

@end
