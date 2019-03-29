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
#import "AutocompleteItem.h"

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

- (void)setTitleWithAutoCompleteItem:(AutocompleteItem *)item
{
	self.textColor = [ConvertHexColor hexCodeToNSColor:item.ProjectColor];
	self.attributedStringValue = [self attributeStringWithAutoCompleteItem:item];
}

- (NSAttributedString *)attributeStringWithItem:(TimeEntryViewItem *)view_item
{
	return [self attributeStringWithClient:view_item.ClientLabel
									taskID:view_item.TaskID
									  task:view_item.TaskLabel
								   project:view_item.ProjectLabel];
}

- (NSAttributedString *)attributeStringWithAutoCompleteItem:(AutocompleteItem *)item
{
	return [self attributeStringWithClient:item.ClientLabel
									taskID:item.TaskID
									  task:item.TaskLabel
								   project:item.ProjectLabel];
}

- (NSAttributedString *)attributeStringWithClient:(NSString *)client taskID:(NSInteger)taskID task:(NSString *)task project:(NSString *)project
{
	NSMutableAttributedString *string;

	if (taskID != 0)
	{
		string = [[NSMutableAttributedString alloc] initWithString:[task stringByAppendingString:@". "]];

		[string setAttributes:
		 @{
			 NSFontAttributeName : [NSFont systemFontOfSize:12],
			 NSForegroundColorAttributeName:[NSColor controlTextColor]
		 }
						range:NSMakeRange(0, [string length])];

		NSMutableAttributedString *projectName = [[NSMutableAttributedString alloc] initWithString:[project stringByAppendingString:@" "]];

		[string appendAttributedString:projectName];
	}
	else
	{
		string = [[NSMutableAttributedString alloc] initWithString:[project stringByAppendingString:@" "]];
	}

	if ([client length] > 0)
	{
		NSString *clientTitle = [NSString stringWithFormat:@"• %@", client];
		NSMutableAttributedString *clientName = [[NSMutableAttributedString alloc] initWithString:clientTitle];

		[clientName setAttributes:
		 @{
			 NSFontAttributeName : [NSFont systemFontOfSize:12],
			 NSForegroundColorAttributeName:[self clientTextColor]
		 }
							range:NSMakeRange(0, [clientName length])];
		[string appendAttributedString:clientName];
	}
	return [string copy];
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
