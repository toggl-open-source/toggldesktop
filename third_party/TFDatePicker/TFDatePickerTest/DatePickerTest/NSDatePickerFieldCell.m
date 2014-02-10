//
//  NSDatePickerFieldCell.m
//  ShootStudio
//
//  Created by Tom Fewster on 16/06/2010.
//  Copyright 2010 Tom Fewster. All rights reserved.
//

#import "NSDatePickerFieldCell.h"
#import "DatePickerPopoverController.h"

@implementation NSDatePickerFieldCell

@synthesize dateButtonCell;

- (id)init {
	return [super init];
}

- (id)initTextCell:(NSString *)aString {
	if ((self = [super initTextCell:aString])) {
		[self setDateButtonCell:[[NSButtonCell alloc] initImageCell:nil]];
		[self resetDateButtonCell];
	}
	return self;
}

- (void)dealloc {
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (id)copyWithZone:(NSZone *)zone {
	NSDatePickerFieldCell *c = [super copyWithZone:zone];
	c->dateButtonCell = [dateButtonCell copyWithZone:zone];
	c->datePickerViewController = datePickerViewController;
	return c;
}

- (BOOL)isOpaque {
	return [super isOpaque] && [dateButtonCell isOpaque];	// only if all components are opaque
}

- (void)drawInteriorWithFrame:(NSRect)cellFrame inView:(NSView*)controlView {
	[super drawInteriorWithFrame:[self searchTextRectForBounds:cellFrame] inView:controlView];
	[dateButtonCell drawInteriorWithFrame:[self cancelButtonRectForBounds:cellFrame] inView:controlView];
}

- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView*)controlView {
	[super drawWithFrame:cellFrame inView:controlView];
	[dateButtonCell drawWithFrame:[self cancelButtonRectForBounds:cellFrame] inView:controlView];
}

- (void) resetDateButtonCell {
	NSButtonCell *c;
	
	c = [self dateButtonCell];
		// configure the button
	[c setButtonType: NSMomentaryChangeButton];
	[c setBezelStyle: NSRegularSquareBezelStyle];
	[c setBordered:NO];
	[c setBezeled:NO];
	[c setEditable:NO];
	[c setImagePosition: NSImageOnly];
	[c setImage:[NSImage imageNamed:@"calendar_full.png"]];
	[c setHighlightsBy:NSContentsCellMask];
		//  [c setAction: [self action]];
		//  [c setTarget: [self target]];
	[c setAction:@selector(performClick:)];
	[c setTarget:self];
	[c sendActionOn:NSLeftMouseUpMask];
		//	[c setKeyEquivalent: @"\r"];
}

- (void)performClick:(id)sender {
	if (!datePickerViewController) {
		datePickerViewController = [[DatePickerPopoverController alloc] init];
	}
	NSRect r = [self cancelButtonRectForBounds:[[self controlView] bounds]];
	
	if ([self isEnabled]) {
		[datePickerViewController showDatePicker:nil relativeToRect:r inView:[self controlView]];
		[[datePickerViewController datePicker] setDelegate:self];
		[datePickerViewController.datePicker setDateValue:[self dateValue]];
		[datePickerViewController.datePicker setDatePickerElements:[self datePickerElements]];
	}
}

- (void)datePickerCell:(NSDatePickerCell *)aDatePickerCell validateProposedDateValue:(NSDate **)proposedDateValue timeInterval:(NSTimeInterval *)proposedTimeInterval {
	//	call the original delegate to validate the proposed time
	//	[[self delegate] datePickerCell:self validateProposedDateValue:proposedDateValue timeInterval:proposedTimeInterval];
	//[(NSDatePicker *)[self controlView] setDateValue:*proposedDateValue];
	//DebugLog(@"Proposing: %@", *proposedDateValue);
	NSDictionary *bindingInfo = [[self controlView] infoForBinding:@"value"];
	NSString *keyPath = [bindingInfo valueForKey:NSObservedKeyPathKey];
	[[self delegate] datePickerCell:self validateProposedDateValue:proposedDateValue timeInterval:proposedTimeInterval];
	[[bindingInfo objectForKey:NSObservedObjectKey] setValue:*proposedDateValue forKeyPath:keyPath];	
}

#define ICON_WIDTH		16
#define ICON_PADDING	3

- (NSRect)cancelButtonRectForBounds:(NSRect)rect {
	NSRect text, clear;
	NSDivideRect(rect, &clear, &text, ICON_WIDTH + ICON_PADDING, NSMaxXEdge);
	clear.origin.y -= ICON_PADDING;
	clear.origin.x -= ICON_PADDING;
	return clear;
}

- (NSRect)searchTextRectForBounds:(NSRect)rect {
	NSRect text, clear;
	NSDivideRect(rect, &clear, &text, ICON_WIDTH + (ICON_PADDING * 2), NSMaxXEdge);
	
	return text;
}

- (BOOL)trackMouse:(NSEvent *)event inRect:(NSRect)cellFrame ofView:(NSView *)controlView untilMouseUp:(BOOL)untilMouseUp {
	// check if we should forward to subcell
	NSPoint loc=[event locationInWindow];
	loc = [controlView convertPoint:loc fromView:nil];
	if(NSMouseInRect(loc, [self cancelButtonRectForBounds:cellFrame], NO)) {
		return [dateButtonCell trackMouse:event inRect:cellFrame ofView:controlView untilMouseUp:untilMouseUp];
	}
 	return [super trackMouse:event inRect:cellFrame ofView:controlView untilMouseUp:untilMouseUp];
}

#pragma mark -
#pragma mark NSCoding protocol

- (void) encodeWithCoder: (NSCoder*)aCoder {
	[super encodeWithCoder: aCoder];
	[aCoder encodeObject:dateButtonCell];
}

- (id)initWithCoder:(NSCoder*)aDecoder {
	self = [super initWithCoder:aDecoder];
	
	if(self != nil) {
		if ([aDecoder allowsKeyedCoding]) {
			[self setDateButtonCell:[aDecoder decodeObjectForKey: @"NSDatePickerFieldCell"]];
		} else {
			[self setDateButtonCell:[aDecoder decodeObject]];
		}
		if (![self dateButtonCell]) {
			[self setDateButtonCell:[[NSButtonCell alloc] initImageCell:nil]];
		}
		[self resetDateButtonCell];
    }
	
	return self;
}


@end
