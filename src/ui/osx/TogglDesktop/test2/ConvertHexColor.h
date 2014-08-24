//
//  ConvertHexColor.h
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 05/11/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

@interface ConvertHexColor : NSObject
+ (NSColor *)hexCodeToNSColor:(NSString *)hexCode;
@end
