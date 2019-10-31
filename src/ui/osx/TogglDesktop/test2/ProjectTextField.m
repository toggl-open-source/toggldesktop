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
#import "TogglDesktop-Swift.h"

@implementation ProjectTextField

- (instancetype)init
{
	self = [super init];
	if (self)
	{
		self.renderClient = YES;
	}
	return self;
}

- (void)awakeFromNib
{
	[super awakeFromNib];

	self.renderClient = YES;
}

- (void)mouseDown:(NSEvent *)theEvent
{
	[self sendAction:@selector(textFieldClicked:) to:[self delegate]];
	[super mouseDown:theEvent];
}

- (void)setAttributedStringValue:(NSAttributedString *)attributedStringValue
{
	[super setAttributedStringValue:attributedStringValue];
	self.toolTip = self.attributedStringValue.string;
}

- (void)setTitleWithTimeEntry:(TimeEntryViewItem *)item
{
	self.textColor = [ConvertHexColor hexCodeToNSColor:item.ProjectColor];
	self.attributedStringValue = [self attributeStringWithItem:item];
}

- (void)setTitleWithAutoCompleteItem:(AutocompleteItem *)item
{
	self.textColor = (item.Type == 1) ? nil : [ConvertHexColor hexCodeToNSColor:item.ProjectColor];
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
	NSMutableParagraphStyle *parStyle = [[NSMutableParagraphStyle alloc] init];

	[parStyle setLineBreakMode:NSLineBreakByTruncatingTail];
	NSColor *color = self.textColor == nil ? [NSColor controlTextColor] : self.textColor;
	NSDictionary *baseAttribute =          @{
			NSFontAttributeName: [NSFont systemFontOfSize:12],
			NSForegroundColorAttributeName: color,
			NSParagraphStyleAttributeName: parStyle
	};
	string = [[NSMutableAttributedString alloc] initWithString:project attributes:baseAttribute];

	if (taskID != 0)
	{
		NSMutableAttributedString *taskName;
		if (self.renderTask)
		{
			string = [[NSMutableAttributedString alloc] initWithString:@"" attributes:baseAttribute];
			taskName = [[NSMutableAttributedString alloc] initWithString:[@"     " stringByAppendingString:task]];
		}
		else
		{
			taskName = [[NSMutableAttributedString alloc] initWithString:[@": " stringByAppendingString:task]];
		}

		[string appendAttributedString:taskName];
	}

	if (self.renderClient && [client length] > 0)
	{
		NSString *clientTitle = [NSString stringWithFormat:@" • %@", client];
		NSMutableAttributedString *clientName = [[NSMutableAttributedString alloc] initWithString:clientTitle];

		[clientName setAttributes:
		 @{
			 NSFontAttributeName : [NSFont systemFontOfSize:12],
			 NSForegroundColorAttributeName:[self clientTextColor],
			 NSParagraphStyleAttributeName: parStyle
		 }
							range:NSMakeRange(0, [clientName length])];
		[string appendAttributedString:clientName];
	}
	return [string copy];
}

- (NSColor *)clientTextColor
{
	if (self.customClientTextColor != nil)
	{
		return self.customClientTextColor;
	}

	if (@available(macOS 10.13, *))
	{
		return [NSColor colorNamed:@"grey-text-color"];
	}
	else
	{
		return [ConvertHexColor hexCodeToNSColor:@"#555555"];
	}
}

- (void)setTextColor:(NSColor *)textColor
{
	NSColor *visibleColor = [textColor visibleColor];

	[super setTextColor:visibleColor];
}

@end
