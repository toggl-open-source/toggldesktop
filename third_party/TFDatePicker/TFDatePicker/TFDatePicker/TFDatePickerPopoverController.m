//
//  DatePickerPopoverController.m
//  ShootStudio
//
//  Created by Tom Fewster on 03/10/2011.
//  Copyright (c) 2011 Tom Fewster. All rights reserved.
//

#import "TFDatePickerPopoverController.h"

@interface TFDatePickerPopoverController ()
@property (strong) IBOutlet NSPopover *popover;
@property (copy) void(^completionHandler)(NSDate *selectedDate);

@property (strong) NSLayoutConstraint *widthConstraint;
@property (strong) NSLayoutConstraint *heightConstraint;

@end

@implementation TFDatePickerPopoverController

@synthesize datePicker = _datePicker;
@synthesize popover = _popover;
@synthesize completionHandler = _completionHandler;
@synthesize widthConstraint = _widthConstraint;
@synthesize heightConstraint = _heightConstraint;

#define DATE_AND_TIME_SIZE NSMakeSize(278.0f, 148.0f)
#define DATE_SIZE NSMakeSize(139.0f, 148.0f)
#define TIME_SIZE NSMakeSize(122.0f, 123.0f)

- (id)init {
	if ((self = [super init])) {
		self.view = [[NSView alloc] initWithFrame:NSMakeRect(0.0, 0.0, 400.0, 180.0)];
		_datePicker = [[NSDatePicker alloc] initWithFrame:NSMakeRect(0.0, 0.0, 400.0, 150.0)];
		_datePicker.datePickerStyle = NSClockAndCalendarDatePickerStyle;
		_datePicker.bezeled = NO;
		_datePicker.target = self;
		_datePicker.action = @selector(dateChanged:);
		[_datePicker setTranslatesAutoresizingMaskIntoConstraints:NO];
		[self.view addSubview:_datePicker];

		NSSize datePickerSize = DATE_AND_TIME_SIZE;
		_widthConstraint = [NSLayoutConstraint constraintWithItem:_datePicker attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1.0f constant:datePickerSize.width];
		_heightConstraint = [NSLayoutConstraint constraintWithItem:_datePicker attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1.0f constant:datePickerSize.height];
		[self.view addConstraint:_widthConstraint];
		[self.view addConstraint:_heightConstraint];

		NSDictionary *views = NSDictionaryOfVariableBindings(_datePicker);
		[self.view addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"H:|-5-[_datePicker]-5-|" options:0 metrics:nil views:views]];
		[self.view addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|-5-[_datePicker]-5-|" options:0 metrics:nil views:views]];
	}

	return self;
}

- (IBAction)showDatePickerRelativeToRect:(NSRect)rect inView:(NSView *)view completionHander:(void(^)(NSDate *selectedDate))completionHandler {
	_completionHandler = completionHandler;

	NSSize datePickerSize = NSZeroSize;
	if ((_datePicker.datePickerElements & NSYearMonthDayDatePickerElementFlag) == NSYearMonthDayDatePickerElementFlag && (_datePicker.datePickerElements & NSHourMinuteDatePickerElementFlag) == NSHourMinuteDatePickerElementFlag) {
		datePickerSize = DATE_AND_TIME_SIZE;
	} else if ((_datePicker.datePickerElements & NSHourMinuteDatePickerElementFlag) == NSHourMinuteDatePickerElementFlag) {
		datePickerSize = TIME_SIZE;
	} else if ((_datePicker.datePickerElements & NSYearMonthDayDatePickerElementFlag) == NSYearMonthDayDatePickerElementFlag) {
		datePickerSize = DATE_SIZE;
	}

	[self.view removeConstraint:_widthConstraint];
	[self.view removeConstraint:_heightConstraint];

	_widthConstraint = [NSLayoutConstraint constraintWithItem:_datePicker attribute:NSLayoutAttributeWidth relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1.0f constant:datePickerSize.width];
	_heightConstraint = [NSLayoutConstraint constraintWithItem:_datePicker attribute:NSLayoutAttributeHeight relatedBy:NSLayoutRelationEqual toItem:nil attribute:NSLayoutAttributeNotAnAttribute multiplier:1.0f constant:datePickerSize.height];
	[self.view addConstraint:_widthConstraint];
	[self.view addConstraint:_heightConstraint];

	_popover = [[NSPopover alloc] init];
	_popover.delegate = self;
	_popover.contentViewController = self;
	_popover.behavior = NSPopoverBehaviorTransient;
	[_popover showRelativeToRect:rect ofView:view preferredEdge:NSMaxXEdge];
}

- (void)popoverWillClose:(NSNotification *)notification {
	_completionHandler(_datePicker.dateValue);
}

- (void)dateChanged:(id)sender {
	_completionHandler(_datePicker.dateValue);
	[_popover close];
}

@end
