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

- (NSAttributedString *)setFormatedText:(AutocompleteItem *)view_item
{
	[self.bottomLine setHidden:(view_item.Type != -3)];
	// Format is: Description - TaskName · ProjectName - ClientName
	NSMutableAttributedString *string;

	AutoCompleteCellType cellType = [AutoCompleteTableCell cellTypeFrom:view_item];

	// Category row
	if (cellType == AutoCompleteCellTypeCategory)
	{
		return [self textForCategoryCellWithViewItem:view_item];
	}

	// Client row / No project row
	if (cellType == AutoCompleteCellTypeClient || cellType == AutoCompleteCellTypeNoProject)
	{
		return [self textForClientCellWithViewItem:view_item];
	}

	// Workspace row
	if (cellType == AutoCompleteCellTypeWorkspace)
	{
		return [self textForWorkspaceCellWithViewItem:view_item];
	}

	// Item rows
	string = [[NSMutableAttributedString alloc] initWithString:view_item.Description];

	[string setAttributes:
	 @{
		 NSFontAttributeName : [NSFont systemFontOfSize:12],
		 NSForegroundColorAttributeName:[NSColor labelColor]
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
				 NSFontAttributeName : [NSFont systemFontOfSize:12],
				 NSForegroundColorAttributeName:[NSColor labelColor]
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
				 NSFontAttributeName : [NSFont systemFontOfSize:12],
				 NSForegroundColorAttributeName:[ConvertHexColor hexCodeToNSColor:view_item.ProjectColor]
			 }
								range:NSMakeRange(0, [projectDot length])];
			[string appendAttributedString:projectDot];

			NSMutableAttributedString *projectName = [[NSMutableAttributedString alloc] initWithString:view_item.ProjectLabel];

			[projectName setAttributes:
			 @{
				 NSFontAttributeName : [NSFont systemFontOfSize:12],
				 NSForegroundColorAttributeName:[ConvertHexColor hexCodeToNSColor:view_item.ProjectColor]
			 }
								 range:NSMakeRange(0, [projectName length])];

			[string appendAttributedString:projectName];
		}
	}

	if (cellType == AutoCompleteCellTypeProject)
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
			 NSFontAttributeName : [NSFont systemFontOfSize:12],
			 NSForegroundColorAttributeName:[NSColor labelColor]
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

- (NSAttributedString *)textForCategoryCellWithViewItem:(AutocompleteItem *)viewItem {
	NSDictionary *attribute =      @{
			NSFontAttributeName : [NSFont systemFontOfSize:11 weight:NSFontWeightMedium],
			NSForegroundColorAttributeName:[self categoryLabelColor]
	};

	return [[NSAttributedString alloc] initWithString:viewItem.Text attributes:attribute];
}

- (NSAttributedString *)textForClientCellWithViewItem:(AutocompleteItem *)viewItem
{
	NSString *spacing = @"  ";
	NSString *text = [NSString stringWithFormat:@"%@%@", spacing, viewItem.Text];
	NSDictionary *attribute = @{
			NSFontAttributeName : [NSFont systemFontOfSize:12],
			NSForegroundColorAttributeName:[NSColor labelColor]
	};

	return [[NSAttributedString alloc] initWithString:text attributes:attribute];
}

- (NSAttributedString *)textForWorkspaceCellWithViewItem:(AutocompleteItem *)viewItem
{
	NSMutableParagraphStyle *paragrapStyle = NSMutableParagraphStyle.new;

	paragrapStyle.alignment = kCTTextAlignmentCenter;
	NSDictionary *attribute =  @{
			NSFontAttributeName : [NSFont systemFontOfSize:14],
			NSForegroundColorAttributeName:[NSColor labelColor],
			NSParagraphStyleAttributeName:paragrapStyle
	};

	return [[NSAttributedString alloc] initWithString:viewItem.Text attributes:attribute];
}

- (NSColor *)categoryLabelColor
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
