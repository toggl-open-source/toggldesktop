//
//  NSBoxClickable.m
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 13/11/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "NSBoxClickable.h"
#import "toggl_api.h"

@implementation NSBoxClickable

extern void *ctx;

- (void)mouseDown:(NSEvent *)event
{
	toggl_edit(ctx, "", true, "");
}

- (BOOL)acceptsFirstMouse:(NSEvent *)theEvent
{
	return YES;
}

@end
