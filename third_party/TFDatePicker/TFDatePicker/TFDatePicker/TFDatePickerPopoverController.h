//
//  DatePickerPopoverController.h
//  ShootStudio
//
//  Created by Tom Fewster on 03/10/2011.
//  Copyright (c) 2011 Tom Fewster. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface TFDatePickerPopoverController : NSViewController <NSPopoverDelegate>;

@property (readonly, strong) IBOutlet NSDatePicker *datePicker;

- (IBAction)showDatePickerRelativeToRect:(NSRect)rect inView:(NSView *)view completionHander:(void(^)(NSDate *selectedDate))completionHandler;

@end