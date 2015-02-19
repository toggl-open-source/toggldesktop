//
//  IdleEvent.h
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 04/11/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface IdleEvent : NSObject
@property (strong) NSString *guid;
@property (strong) NSString *since;
@property (strong) NSString *duration;
@property uint64_t started;
@property (strong) NSString *timeEntryDescription;
@end
