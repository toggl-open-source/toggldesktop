//
//  AutoCompleteTableContainer.m
//  TogglDesktop
//
//  Created by Indrek Vändrik on 09/03/2018.
//  Copyright © 2018 Alari. All rights reserved.
//

#import "AutoCompleteTableContainer.h"

@interface AutoCompleteTableContainer ()
@property (strong, nonatomic) NSShadow *dropShadow;
@end

@implementation AutoCompleteTableContainer

- (instancetype)initWithFrame:(CGRect)frame
{
	self = [super initWithFrame:frame];
	if (self)
	{
		[self setWantsLayer:YES];
		self.layer.masksToBounds = YES;
		self.dropShadow = [[NSShadow alloc] init];
		[self.dropShadow setShadowColor:[[NSColor blackColor]
										 colorWithAlphaComponent:0.13f]];
		[self.dropShadow setShadowOffset:NSMakeSize(0, -2)];
		[self.dropShadow setShadowBlurRadius:6.0];
	}
	return self;
}

- (void)drawRect:(NSRect)dirtyRect
{
	[super drawRect:dirtyRect];
	[self.window.contentView setWantsLayer:YES];
	[self setWantsLayer:YES];
	[self setShadow:self.dropShadow];
}

@end
