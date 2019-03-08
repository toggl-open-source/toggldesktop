//
//  AutoCompleteTableCell.m
//  LiteComplete
//
//  Created by Indrek Vändrik on 20/02/2018.
//  Copyright © 2018 Toggl. All rights reserved.
//

#import "AutoCompleteTableCell.h"
#import "TogglDesktop-Swift.h"

@implementation AutoCompleteTableCell

- (void)render:(AutocompleteItem *)view_item selected:(BOOL)selected
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	[self setFocused:selected];
	[self.cellDescription setAttributedStringValue:[self setFormatedText:view_item]];
	self.cellDescription.toolTip = view_item.Text;
	self.isSelectable = view_item.Type > -1;
}

- (void)setFocused:(BOOL)focus
{
	NSColor *color = [NSColor textBackgroundColor];

	if (focus == YES)
	{
		color = [NSColor selectedTextBackgroundColor];
	}
	[self.backgroundBox setFillColor:color];
}

- (NSMutableAttributedString *)setFormatedText:(AutocompleteItem *)view_item
{
	[self.bottomLine setHidden:(view_item.Type != -3)];
	// Format is: Description - TaskName · ProjectName - ClientName
	NSMutableAttributedString *string;

	AutoCompleteCellType cellType = [self cellTypeFrom:view_item];

	// Category row
	if (cellType == AutoCompleteCellTypeCategory)
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
	if (cellType == AutoCompleteCellTypeClient || cellType == AutoCompleteCellTypeNoProject)
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

	// Workspace row
	if (cellType == AutoCompleteCellTypeWorkspace)
	{
		NSMutableParagraphStyle *paragrapStyle = NSMutableParagraphStyle.new;
		paragrapStyle.alignment = kCTTextAlignmentCenter;

		string = [[NSMutableAttributedString alloc] initWithString:view_item.Text];

		[string setAttributes:
		 @{
			 NSFontAttributeName : [NSFont boldSystemFontOfSize:12],
			 NSForegroundColorAttributeName:[NSColor disabledControlTextColor],
			 NSParagraphStyleAttributeName:paragrapStyle
		 }
						range:NSMakeRange(0, [string length])];
		return string;
	}

	// Item rows
	string = [[NSMutableAttributedString alloc] initWithString:view_item.Description];

	[string setAttributes:
	 @{
		 NSFontAttributeName : [NSFont systemFontOfSize:[NSFont systemFontSize]],
		 NSForegroundColorAttributeName:[NSColor controlTextColor]
	 }
					range:NSMakeRange(0, [string length])];

	BOOL addProjectLabel = NO;
	if (view_item.ProjectID != 0)
	{
		if (view_item.TaskID != 0)
		{
			if ([string length] > 0)
			{
				[string appendAttributedString:[[NSMutableAttributedString alloc] initWithString:@" - "]];
				// Project label should be added when in Time Entries category
				addProjectLabel = YES;
			}
			else
			{
				[string appendAttributedString:[[NSMutableAttributedString alloc] initWithString:@"        "]];
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
		else
		{
			addProjectLabel = YES;
		}
		if (addProjectLabel)
		{
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
	}

	if (cellType == AutoCompleteCellTypeTimeEntryWithProject)
	{
		// Add more padding to the front of project items
		NSMutableAttributedString *result = [[NSMutableAttributedString alloc] initWithString:@"      "];
		[result appendAttributedString:string];

		return result;
	}

	// For time entries show all params: description, task, project, client
	if (cellType == AutoCompleteCellTypeTimeEntryFullData)
	{
		NSMutableAttributedString *clientName = [[NSMutableAttributedString alloc] initWithString:view_item.ClientLabel];

		[clientName setAttributes:
		 @{
			 NSFontAttributeName : [NSFont systemFontOfSize:[NSFont systemFontSize]],
			 NSForegroundColorAttributeName:[NSColor disabledControlTextColor]
		 }
							range:NSMakeRange(0, [clientName length])];

		NSMutableAttributedString *space = [[NSMutableAttributedString alloc] initWithString:@" "];
		[string appendAttributedString:space];
		[string appendAttributedString:clientName];
	}

	// Add padding to the front of regular items
	NSMutableAttributedString *result = [[NSMutableAttributedString alloc] initWithString:@"  "];
	[result appendAttributedString:string];

	return result;
}

@end
