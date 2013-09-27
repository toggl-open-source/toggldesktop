//
//  TimeEntryViewItem.h
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 25/09/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "kopsik_api.h"

@interface TimeEntryViewItem : NSObject
- (void)load:(KopsikTimeEntryViewItem *)data;
@property int duration_in_seconds;
@property NSString *description;
@property NSString *project;
@property NSString *duration;
@property NSString *color;
@end
