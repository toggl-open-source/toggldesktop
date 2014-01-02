//
//  User.h
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 02/10/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "kopsik_api.h"

@interface User : NSObject
- (void)load:(KopsikUser *)data;
@property long ID;
@property (strong) NSString *fullname;
@property BOOL recordTimeline;
@end
