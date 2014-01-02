//
//  DateHeader.h
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 11/11/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface DateHeader : NSObject
@property (strong) NSString *date;
@property (strong) NSString *duration;
@property (strong) NSDate *started;
- (NSString *)description;
@end
