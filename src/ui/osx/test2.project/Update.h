//
//  Update.h
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 29/10/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "kopsik_api.h"

@interface Update : NSObject
- (void)load:(KopsikUpdateViewItem *)view;
@property BOOL is_checking;
@property BOOL is_update_available;
@property (strong) NSString *channel;
@property (strong) NSString *version;
@property (strong) NSString *URL;
@end
