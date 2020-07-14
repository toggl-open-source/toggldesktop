//
//  ConvertHexColor.h
//  Toggl Track on the Mac
//
//  Created by Tanel Lebedev on 05/11/2013.
//  Copyright (c) 2013 TogglTrack developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface ConvertHexColor : NSObject
+ (NSColor *)hexCodeToNSColor:(NSString *)hexCode;
@end
