//
//  AppDelegate.h
//  DatePickerTest
//
//  Created by Tom Fewster on 18/09/2012.
//  Copyright (c) 2012 Tom Fewster. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface AppDelegate : NSObject <NSApplicationDelegate, NSDatePickerCellDelegate>

@property (assign) IBOutlet NSWindow *window;

@property (nonatomic, strong) NSDate *date1;
@property (strong) NSDate *date2;
@property (nonatomic, assign) BOOL allDay;

@end
