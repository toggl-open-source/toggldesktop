//
//  ConvertHexColor.h
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 05/11/2013.
//  Copyright (c) 2013 Alari. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface ConvertHexColor : NSObject
+ (NSColor *)hexCodeToNSColor:(NSString *)hexCode;
@end
