//
//  NSViewEscapable.m
//  TogglDesktop
//
//  Created by Indrek Vändrik on 22/05/14.
//  Copyright (c) 2014 Alari. All rights reserved.
//

#import "NSViewEscapable.h"
#import "DisplayCommand.h"
#import "UIEvents.h"

@implementation NSViewEscapable

- (void)cancelOperation:(id)sender
{
	[[NSNotificationCenter defaultCenter] postNotificationName:kForceCloseEditPopover
														object:nil];
}

@end
