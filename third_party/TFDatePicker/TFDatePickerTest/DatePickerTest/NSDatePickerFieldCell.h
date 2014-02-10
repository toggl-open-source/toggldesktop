//
//  NSDatePickerFieldCell.h
//  ShootStudio
//
//  Created by Tom Fewster on 16/06/2010.
//  Copyright 2010 Tom Fewster. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class DatePickerPopoverController;

@interface NSDatePickerFieldCell : NSDatePickerCell <NSDatePickerCellDelegate> {
	NSButtonCell *dateButtonCell;
	DatePickerPopoverController *datePickerViewController;

}

@property (nonatomic, strong) NSButtonCell *dateButtonCell;

- (NSRect) cancelButtonRectForBounds:(NSRect) rect;
- (void) resetDateButtonCell;
- (NSRect) searchTextRectForBounds:(NSRect) rect;

@end
