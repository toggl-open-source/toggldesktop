//
//  AutoCompleteTableCell.m
//  LiteComplete
//
//  Created by Indrek Vändrik on 20/02/2018.
//  Copyright © 2018 Toggl. All rights reserved.
//

#import "AutoCompleteTableCell.h"

@implementation AutoCompleteTableCell

- (void)drawRect:(NSRect)dirtyRect
{
	[super drawRect:dirtyRect];

	// Drawing code here.
}

- (void)render:(AutocompleteItem *)view_item
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	[self.cellDescription setAttributedStringValue:[self setFormatedText:view_item]];
	self.cellDescription.toolTip = view_item.Text;
	self.isSelectable = (view_item.Type != -1 && view_item.Type != -2);
}

- (void)setFocused:(BOOL)focus
{
	NSString *color = @"#ffffff";

	if (focus == YES)
	{
		color = @"#f4f4f4";
	}
	[self.backgroundBox setFillColor:[ConvertHexColor hexCodeToNSColor:color]];
}

- (NSMutableAttributedString *)setFormatedText:(AutocompleteItem *)view_item
{
	// Format is: Description - TaskName · ProjectName - ClientName
	NSMutableAttributedString *string;

	// Category row
	if (view_item.Type == -1)
	{
		string = [[NSMutableAttributedString alloc] initWithString:view_item.Text];

		[string setAttributes:
		 @{
			 NSFontAttributeName : [NSFont systemFontOfSize:11],
			 NSForegroundColorAttributeName:[NSColor disabledControlTextColor]
		 }
						range:NSMakeRange(0, [string length])];
		return string;
	}

	// Client row / No project row
	if (view_item.Type == -2 ||  (view_item.Type == 2 && view_item.ProjectID == 0))
	{
		string = [[NSMutableAttributedString alloc] initWithString:view_item.Text];

		[string setAttributes:
		 @{
			 NSFontAttributeName : [NSFont systemFontOfSize:[NSFont systemFontSize]],
			 NSForegroundColorAttributeName:[NSColor disabledControlTextColor]
		 }
						range:NSMakeRange(0, [string length])];

		NSMutableAttributedString *result = [[NSMutableAttributedString alloc] initWithString:@"  "];
		[result appendAttributedString:string];

		return result;
	}

	// Item rows
	string = [[NSMutableAttributedString alloc] initWithString:view_item.Description];

	[string setAttributes:
	 @{
		 NSFontAttributeName : [NSFont systemFontOfSize:[NSFont systemFontSize]],
		 NSForegroundColorAttributeName:[NSColor controlTextColor]
	 }
					range:NSMakeRange(0, [string length])];

	if (view_item.ProjectID != 0)
	{
		if (view_item.TaskID != 0)
		{
			if ([string length] > 0)
			{
				[string appendAttributedString:[[NSMutableAttributedString alloc] initWithString:@" -"]];
			}
			NSMutableAttributedString *task = [[NSMutableAttributedString alloc] initWithString:view_item.TaskLabel];

			[task setAttributes:
			 @{
				 NSFontAttributeName : [NSFont systemFontOfSize:[NSFont systemFontSize]],
				 NSForegroundColorAttributeName:[NSColor controlTextColor]
			 }
						  range:NSMakeRange(0, [task length])];
			[string appendAttributedString:task];
		}
		if ([string length] > 0)
		{
			[string appendAttributedString:[[NSMutableAttributedString alloc] initWithString:@" "]];
		}

		NSMutableAttributedString *projectDot = [[NSMutableAttributedString alloc] initWithString:@"•"];

		[projectDot setAttributes:
		 @{
			 NSFontAttributeName : [NSFont systemFontOfSize:[NSFont systemFontSize]],
			 NSForegroundColorAttributeName:[ConvertHexColor hexCodeToNSColor:view_item.ProjectColor]
		 }
							range:NSMakeRange(0, [projectDot length])];
		[string appendAttributedString:projectDot];

		NSMutableAttributedString *projectName = [[NSMutableAttributedString alloc] initWithString:view_item.ProjectLabel];

		[projectName setAttributes:
		 @{
			 NSFontAttributeName : [NSFont systemFontOfSize:[NSFont systemFontSize]],
			 NSForegroundColorAttributeName:[ConvertHexColor hexCodeToNSColor:view_item.ProjectColor]
		 }
							 range:NSMakeRange(0, [projectName length])];

		[string appendAttributedString:projectName];
	}

	if (view_item.Type == 2)
	{
		// Add more padding to the front of project items
		NSMutableAttributedString *result = [[NSMutableAttributedString alloc] initWithString:@"      "];
		[result appendAttributedString:string];

		return result;
	}

	// Add padding to the front of regular items
	NSMutableAttributedString *result = [[NSMutableAttributedString alloc] initWithString:@"  "];
	[result appendAttributedString:string];

	return result;
}

@end
