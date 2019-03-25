//
//  ProjectTextField.m
//  TogglDesktop
//
//  Created by Nghia Tran on 3/5/19.
//  Copyright © 2019 Alari. All rights reserved.
//

#import "ProjectTextField.h"
#import "TimeEntryViewItem.h"
#import "ConvertHexColor.h"

@implementation ProjectTextField

- (void)mouseDown:(NSEvent *)theEvent
{
	if (self.isInTimerBar)
	{
		[self sendAction:@selector(textFieldClicked:) to:[self delegate]];
	}
	else
	{
		[super mouseDown:theEvent];
	}
}

- (void)setTitleWithTimeEntry:(TimeEntryViewItem *)item
{
	self.textColor = [ConvertHexColor hexCodeToNSColor:item.ProjectColor];
	self.attributedStringValue = [self attributeStringWithItem:item];
}

- (NSMutableAttributedString *)attributeStringWithItem:(TimeEntryViewItem *)view_item
{
	NSMutableAttributedString *string;

	if (view_item.TaskID != 0)
	{
		string = [[NSMutableAttributedString alloc] initWithString:[view_item.TaskLabel stringByAppendingString:@". "]];

		[string setAttributes:
		 @{
			 NSFontAttributeName : [NSFont systemFontOfSize:12],
			 NSForegroundColorAttributeName:[NSColor controlTextColor]
		 }
						range:NSMakeRange(0, [string length])];

		NSMutableAttributedString *projectName = [[NSMutableAttributedString alloc] initWithString:[view_item.ProjectLabel stringByAppendingString:@" "]];

		[string appendAttributedString:projectName];
	}
	else
	{
		string = [[NSMutableAttributedString alloc] initWithString:[view_item.ProjectLabel stringByAppendingString:@" "]];
	}

	if ([view_item.ClientLabel length] > 0)
	{
		NSString *clientTitle = [NSString stringWithFormat:@"• %@", view_item.ClientLabel];
		NSMutableAttributedString *clientName = [[NSMutableAttributedString alloc] initWithString:clientTitle];

		[clientName setAttributes:
		 @{
			 NSFontAttributeName : [NSFont systemFontOfSize:12],
			 NSForegroundColorAttributeName:[self clientTextColor]
		 }
							range:NSMakeRange(0, [clientName length])];
		[string appendAttributedString:clientName];
	}

	return string;
}

- (NSColor *)clientTextColor
{
	if (@available(macOS 10.13, *))
	{
		return [NSColor colorNamed:@"grey-text-color"];
	}
	else
	{
		return [ConvertHexColor hexCodeToNSColor:@"#555555"];
	}
}

@end
