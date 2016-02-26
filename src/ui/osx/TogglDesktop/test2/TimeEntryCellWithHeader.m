//
//  TimeEntryCellWithHeader.m
//  Toggl Desktop on the Mac
//
//  Created by Tambet Masik on 9/26/13.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "TimeEntryCellWithHeader.h"
#import "UIEvents.h"
#import "ConvertHexColor.h"

@implementation TimeEntryCellWithHeader

- (void)render:(TimeEntryViewItem *)view_item
{
	[super render:view_item];
	self.dateDurationTextField.stringValue = view_item.dateDuration;
	self.formattedDateTextField.stringValue = view_item.formattedDate;
}

- (NSArray *)draggingImageComponents
{
	NSMutableArray *result = [NSMutableArray array];

	// Snapshot the main nsbox view and add it in
	NSRect viewBounds = [self.backgroundBox bounds];
	NSBitmapImageRep *imageRep = [self.backgroundBox bitmapImageRepForCachingDisplayInRect:viewBounds];

	[self.backgroundBox cacheDisplayInRect:viewBounds toBitmapImageRep:imageRep];

	NSImage *draggedImage = [[NSImage alloc] initWithSize:[imageRep size]];
	[draggedImage addRepresentation:imageRep];

	// Add in another component
	NSDraggingImageComponent *colorComponent = [NSDraggingImageComponent draggingImageComponentWithKey:@"Cell"];
	colorComponent.contents = draggedImage;

	// Convert the frame to our coordinate system
	viewBounds = [self convertRect:viewBounds fromView:self.backgroundBox];
	colorComponent.frame = viewBounds;

	[result addObject:colorComponent];
	return result;
}

@end
