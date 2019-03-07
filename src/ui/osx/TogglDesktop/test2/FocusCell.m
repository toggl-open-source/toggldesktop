//
//  FocusCell.m
//  TogglDesktop
//
//  Created by Nghia Tran on 3/7/19.
//  Copyright © 2019 Alari. All rights reserved.
//

#import "FocusCell.h"

@implementation FocusCell

+ (CGRect)frameFocusRingFromFrame:(CGRect)frame
{
	NSRect bounds = frame;
	NSRect outerRect = NSMakeRect(bounds.origin.x - 4,
								  bounds.origin.y - 3,
								  bounds.size.width + 8,
								  bounds.size.height + 6);

	return outerRect;
}

- (void)drawFocusRingMaskWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
	CGRect frame = [FocusCell frameFocusRingFromFrame:cellFrame];

	return [super drawFocusRingMaskWithFrame:frame inView:controlView];
}

@end
