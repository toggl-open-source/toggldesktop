//
//  AppDelegate.m
//  DatePickerTest
//
//  Created by Tom Fewster on 18/09/2012.
//  Copyright (c) 2012 Tom Fewster. All rights reserved.
//

#import "AppDelegate.h"

@interface AppDelegate ()
@property (weak) IBOutlet NSDatePicker *datePicker1;
@property (weak) IBOutlet NSDatePicker *datePicker2;
@end

@implementation AppDelegate

@synthesize allDay = _allDay;
@synthesize datePicker1 = _datePicker1;
@synthesize datePicker2 = _datePicker2;
@synthesize date1 = _date1;
@synthesize date2 = _date2;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	// Insert code here to initialize your application
//	_datePicker1.dateValue = [NSDate date];
	_datePicker1.delegate = self;
//	_datePicker2.dateValue = [NSDate date];
	_datePicker2.delegate = self;

	self.date1 = [NSDate date];
	self.date2 = [NSDate date];
	self.allDay = YES;
}

- (void)setAllDay:(BOOL)allDay {
	_allDay = allDay;
	if (!allDay) {
		_datePicker1.datePickerElements = NSYearMonthDayDatePickerElementFlag | NSHourMinuteDatePickerElementFlag;
		_datePicker2.datePickerElements = NSYearMonthDayDatePickerElementFlag | NSHourMinuteDatePickerElementFlag;
	} else {
		_datePicker1.datePickerElements = NSYearMonthDayDatePickerElementFlag;
		_datePicker2.datePickerElements = NSYearMonthDayDatePickerElementFlag;
	}
}

const NSTimeInterval timeStep = 5.0f * 60.0f; // 5 mins

- (void)datePickerCell:(NSDatePickerCell *)aDatePickerCell validateProposedDateValue:(NSDate **)proposedDateValue timeInterval:(NSTimeInterval *)proposedTimeInterval {
	NSDate *date = *proposedDateValue;
//	*proposedTimeInterval = (double)(((NSUInteger)*proposedTimeInterval) % 15);
	NSTimeInterval timeInterval = [date timeIntervalSinceReferenceDate] - (timeStep/2.0);

	if(timeStep != 0) {
		double remainder = fmod(timeInterval, timeStep);
		if (remainder != 0.0f) {
			timeInterval = timeInterval - remainder;
		}
	}

	*proposedDateValue = [NSDate dateWithTimeIntervalSinceReferenceDate:timeInterval];
}

- (void)setDate1:(NSDate *)date1 {
	[self willChangeValueForKey:@"date1"];
	_date1 = date1;
	[self didChangeValueForKey:@"date1"];
}

@end
