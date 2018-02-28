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

	self.cellDescription.stringValue = view_item.Text;
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

@end
