//
//  ClickableImageView.m
//  TogglDesktop
//
//  Created by Nghia Tran on 9/16/19.
//  Copyright © 2019 Alari. All rights reserved.
//

#import "ClickableImageView.h"

@implementation ClickableImageView

- (void)mouseDown:(NSEvent *)theEvent
{
	[self.delegate imageViewOnClick:self];
}

@end
