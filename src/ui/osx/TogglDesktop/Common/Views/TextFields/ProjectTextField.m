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
#import "IdleEvent.h"
#import "Toggl_Track-Swift.h"

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
    if ([self.delegate respondsToSelector:@selector(textFieldClicked:)]) {
        [self sendAction:@selector(textFieldClicked:) to:[self delegate]];
    } else {
        [super mouseDown:theEvent];
    }
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

- (void)setTitleWithIdleEvent:(IdleEvent *)item
{
	self.textColor = item.projectColor;
	self.attributedStringValue = [self attributeStringWithIdleEvent:item];
}

- (NSAttributedString *)attributeStringWithItem:(TimeEntryViewItem *)view_item
{
	return [self attributeStringWithClient:view_item.ClientLabel
									  task:view_item.TaskLabel
								   project:view_item.ProjectLabel];
}

- (NSAttributedString *)attributeStringWithAutoCompleteItem:(AutocompleteItem *)item
{
	return [self attributeStringWithClient:item.ClientLabel
									  task:item.TaskLabel
								   project:item.ProjectLabel];
}

- (NSAttributedString *)attributeStringWithIdleEvent:(IdleEvent *)item
{
	return [self attributeStringWithClient:nil
									  task:item.taskName
								   project:item.projectName];
}

- (NSAttributedString *)attributeStringWithClient:(NSString *)client task:(nullable NSString *)task project:(NSString *)project
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

	if (task.length > 0)
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
    return [NSColor greyText];
}

- (void)setTextColor:(NSColor *)textColor
{
    NSColor *adaptiveColor = [textColor getAdaptiveColorForText];
	[super setTextColor:adaptiveColor];
}

@end
