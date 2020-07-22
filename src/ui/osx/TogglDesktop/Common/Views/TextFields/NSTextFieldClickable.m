//
//  NSTextFieldClickable.m
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 11/11/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "NSTextFieldClickable.h"
#import "UIEvents.h"
#import "TogglDesktop-Swift.h"

@interface NSTextFieldClickable () <NSTextFieldDelegate>
@end

@implementation NSTextFieldClickable

- (void)awakeFromNib
{
	[super awakeFromNib];
	[self initDefaultValue];
}

- (void)initDefaultValue
{
	self.titleUnderline = NO;
}

- (void)mouseDown:(NSEvent *)theEvent
{
	if (self.isEditable)
	{
		[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kForceCloseEditPopover
																	object:nil];
		return;
	}
	[self sendAction:@selector(textFieldClicked:) to:[self delegate]];
}

- (void)setTitleUnderline:(BOOL)titleUnderline {
	_titleUnderline = titleUnderline;
	if (_titleUnderline)
	{
		[self drawUnderline];
	}
}

- (void)drawUnderline {
	NSDictionary<NSAttributedStringKey, id> *attributes = @{ NSFontAttributeName: self.font,
															 NSForegroundColorAttributeName: self.textColor,
															 NSUnderlineStyleAttributeName: @(NSUnderlineStyleSingle), };
	NSAttributedString *underlineString = [[NSAttributedString alloc] initWithString:self.stringValue
																		  attributes:attributes];

	self.attributedStringValue = underlineString;
}

@end
