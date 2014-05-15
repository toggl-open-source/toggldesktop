//
//  NSTextFieldClickablePointer.m
//  TogglDesktop
//
//  Created by Indrek Vändrik on 14/05/14.
//  Copyright (c) 2014 Alari. All rights reserved.
//

#import "NSTextFieldClickablePointer.h"

@interface NSTextFieldClickablePointer ()
@property (nonatomic, strong) NSTrackingArea *trackingArea;
@end

@implementation NSTextFieldClickablePointer

- (id)initWithFrame:(NSRect)frame
{
	self = [super initWithFrame:frame];
	if (self)
	{
		_trackingArea = [[NSTrackingArea alloc]initWithRect:[self bounds] options:(NSTrackingMouseMoved | NSTrackingActiveInKeyWindow) owner:self userInfo:nil];
		[self addTrackingArea:_trackingArea];
	}
	return self;
}

- (void)mouseMoved:(NSEvent *)event
{
	if ([self isEditable])
	{
		[[NSCursor IBeamCursor] set];
	}
	else
	{
		[[NSCursor pointingHandCursor] set];
	}
}

- (void)updateTrackingAreas
{
	[super updateTrackingAreas];
	[self removeTrackingArea:_trackingArea];
	_trackingArea = [[NSTrackingArea alloc] initWithRect:[self bounds] options:(NSTrackingMouseMoved | NSTrackingActiveInKeyWindow) owner:self userInfo:nil];
	[self addTrackingArea:_trackingArea];
}

@end
