//
//  NSBoxClickable.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 13/11/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "NSBoxClickable.h"
#import "kopsik_api.h"

@implementation NSBoxClickable

extern void *ctx;

- (void)mouseDown:(NSEvent *)event
{
	kopsik_edit(ctx, "", true, "");
}

- (BOOL)acceptsFirstMouse:(NSEvent *)theEvent
{
	return YES;
}

@end
