//
//  TimeEntryListViewController+Drag.m
//  TogglDesktop
//
//  Created by Nghia Tran on 2/27/19.
//  Copyright © 2019 Alari. All rights reserved.
//

#import "TimeEntryListViewController+Drag.h"
#import "TogglDesktop-Swift.h"
#import "TimeEntryListViewController.h"

@implementation TimeEntryListViewController (Drag)

extern void *ctx;

- (BOOL)collectionView:(NSCollectionView *)collectionView writeItemsAtIndexPaths:(NSSet<NSIndexPath *> *)indexPaths toPasteboard:(NSPasteboard *)pasteboard
{
	NSIndexPath *indexPath = indexPaths.allObjects.firstObject;

	if (indexPath == nil)
	{
		return NO;
	}

	TimeEntryViewItem *model = [self.dataSource objectAt:indexPath];
	if (model.Group)
	{
		return NO;
	}
	NSData *data = [NSKeyedArchiver archivedDataWithRootObject:indexPath];
	[pasteboard declareTypes:[NSArray arrayWithObject:NSStringPboardType] owner:self];
	[pasteboard setData:data forType:NSStringPboardType];
	return YES;
}

- (NSDragOperation)collectionView:(NSCollectionView *)collectionView validateDrop:(id<NSDraggingInfo>)draggingInfo proposedIndex:(NSInteger *)proposedDropIndex dropOperation:(NSCollectionViewDropOperation *)proposedDropOperation
{
	return NSDragOperationMove;
}

- (BOOL)collectionView:(NSCollectionView *)collectionView acceptDrop:(id<NSDraggingInfo>)draggingInfo indexPath:(NSIndexPath *)indexPath dropOperation:(NSCollectionViewDropOperation)dropOperation
{
	NSPasteboard *pboard = [draggingInfo draggingPasteboard];
	NSData *rowData = [pboard dataForType:NSStringPboardType];
	NSIndexPath *moveIndexPath = [NSKeyedUnarchiver unarchiveObjectWithData:rowData];

	if (([draggingInfo draggingSource] == collectionView) && [indexPath isEqualTo:moveIndexPath])
	{
		// Updating the dropped item date
		TimeEntryViewItem *dateModel = [self.dataSource objectAt:indexPath];
		TimeEntryViewItem *currentModel = [self.dataSource objectAt:moveIndexPath];

		NSCalendar *calendar = [NSCalendar currentCalendar];
		NSDateComponents *components = [calendar components:(NSCalendarUnitHour | NSCalendarUnitMinute) fromDate:currentModel.started];
		NSInteger hours = [components hour];
		NSInteger minutes = [components minute];
		NSInteger seconds = [components second];

		unsigned unitFlags = NSCalendarUnitYear | NSCalendarUnitMonth |  NSCalendarUnitDay;
		NSDateComponents *comps = [calendar components:unitFlags fromDate:dateModel.started];
		comps.hour = hours;
		comps.minute = minutes;
		comps.second = seconds;
		NSDate *newDate = [calendar dateFromComponents:comps];

		toggl_set_time_entry_date(ctx,
								  [currentModel.GUID UTF8String],
								  [newDate timeIntervalSince1970]);
	}
	return YES;
}

- (void)collectionView:(NSCollectionView *)collectionView draggingSession:(NSDraggingSession *)session willBeginAtPoint:(NSPoint)screenPoint forItemsAtIndexPaths:(NSSet<NSIndexPath *> *)indexPaths
{
	NSIndexPath *indexPath = indexPaths.allObjects.firstObject;

	if (indexPath == nil)
	{
		return;
	}

	NSCollectionViewItem *item = [collectionView itemAtIndexPath:indexPath];
	if (![item isKindOfClass:[TimeEntryCell class]])
	{
		return;
	}

	TimeEntryCell *cellView = (TimeEntryCell *)item;
	if (cellView)
	{
		[session enumerateDraggingItemsWithOptions:NSDraggingItemEnumerationConcurrent
										   forView:collectionView
										   classes:[NSArray arrayWithObject:[NSPasteboardItem class]]
									 searchOptions:@{}
										usingBlock:^(NSDraggingItem *_Nonnull draggingItem, NSInteger idx, BOOL *_Nonnull stop)
		 {
		     // prepare context
			 NSGraphicsContext *theContext = [NSGraphicsContext currentContext];
			 [theContext saveGraphicsState];

		     // drag image needs to be larger than the content in order to encapsulate the drop shadow
			 CGFloat imageOffset = 5;

		     // supply a drag background image
			 NSSize contentSize = draggingItem.draggingFrame.size;
			 contentSize.height = 56;
			 NSSize imageSize = NSMakeSize(contentSize.width + imageOffset, contentSize.height + imageOffset);
			 NSImage *image = [[NSImage alloc] initWithSize:imageSize];
			 [image lockFocus];

		     // define a shadow
			 NSShadow *shadow = [NSShadow new];
			 shadow.shadowColor = [[NSColor lightGrayColor] colorWithAlphaComponent:0.2];
			 shadow.shadowOffset = NSMakeSize(imageOffset, -imageOffset);
			 shadow.shadowBlurRadius = 3;
			 [shadow set];

		     // define content frame
			 NSRect contentFrame = NSMakeRect(0, imageOffset, contentSize.width, contentSize.height);
			 NSBezierPath *contentPath = [NSBezierPath bezierPathWithRect:contentFrame];

		     // draw content border and shadow
			 [[[NSColor lightGrayColor] colorWithAlphaComponent:0.6] set];
			 [contentPath stroke];
			 [theContext restoreGraphicsState];

		     // fill content
			 [[NSColor whiteColor] set];
			 contentPath = [NSBezierPath bezierPathWithRect:NSInsetRect(contentFrame, 1, 1)];
			 [contentPath fill];

			 [image unlockFocus];

		     // update the dragging item frame to accomodate larger image
			 draggingItem.draggingFrame = NSMakeRect(draggingItem.draggingFrame.origin.x, draggingItem.draggingFrame.origin.y, imageSize.width, imageSize.height);

		     // define additional image component for drag
			 NSDraggingImageComponent *backgroundImageComponent = [NSDraggingImageComponent draggingImageComponentWithKey:@"background"];
			 backgroundImageComponent.contents = image;
			 backgroundImageComponent.frame = NSMakeRect(0, 0, imageSize.width, imageSize.height);

		     // we can provide custom content by overridding NSTableViewCell -draggingImageComponents
		     // which defaults to only including the image and text fields
			 draggingItem.imageComponentsProvider = ^NSArray *(void) {
				 NSMutableArray *components = [NSMutableArray arrayWithArray:@[backgroundImageComponent]];
				 NSArray *cellViewComponents = cellView.draggingImageComponents;
				 [cellViewComponents enumerateObjectsUsingBlock:^(NSDraggingImageComponent *component, NSUInteger idx, BOOL *stop) {
					  component.frame = NSMakeRect(component.frame.origin.x, component.frame.origin.y + imageOffset, component.frame.size.width, component.frame.size.height);
				  }];

				 [components addObjectsFromArray:cellViewComponents];
				 return components;
			 };
		 }];
	}
}

@end
