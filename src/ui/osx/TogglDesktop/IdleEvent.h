//
//  IdleEvent.h
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 04/11/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface IdleEvent : NSObject
@property (nonatomic, assign) NSInteger started;
@property (nonatomic, copy) NSString *guid;
@property (nonatomic, copy) NSString *since;
@property (nonatomic, copy) NSString *duration;
@property (nonatomic, copy) NSString *timeEntryDescription;
@end
