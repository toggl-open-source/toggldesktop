//
//  Update.h
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 29/10/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "toggl_api.h"

@interface Update : NSObject
- (void)load:(TogglUpdateView *)view;
@property BOOL is_checking;
@property BOOL is_update_available;
@property (strong) NSString *channel;
@property (strong) NSString *version;
@property (strong) NSString *URL;
@end
