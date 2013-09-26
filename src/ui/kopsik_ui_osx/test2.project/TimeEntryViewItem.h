//
//  TimeEntryViewItem.h
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 25/09/2013.
//  Copyright (c) 2013 Alari. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "kopsik_api.h"

@interface TimeEntryViewItem : NSObject
- (void)load:(TogglTimeEntryViewItem *)data;
@property NSString *description;
@property NSString *project;
@property NSString *duration;
@property NSString *color;
@end
