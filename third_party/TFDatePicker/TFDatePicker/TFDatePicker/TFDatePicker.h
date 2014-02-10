//
//  DatePickerTextField.h
//  ShootStudio
//
//  Created by Tom Fewster on 16/06/2010.
//  Copyright 2010 Tom Fewster. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface TFDatePicker : NSDatePicker <NSDatePickerCellDelegate>
@property id listener;
@end
