//
//  DisplayCommand.h
//  TogglDesktop
//
//  Created by Tanel Lebedev on 13/05/14.
//  Copyright (c) 2014 Alari. All rights reserved.
//

#import <Foundation/Foundation.h>

@class Settings;
@class TimeEntryViewItem;

@interface DisplayCommand : NSObject
@property (nonatomic, assign) BOOL open;
@property (nonatomic, strong) NSMutableArray *timeEntries;
@property (nonatomic, strong) TimeEntryViewItem *timeEntry;
@property (nonatomic, strong) Settings *settings;
@property (nonatomic, assign) NSInteger user_id;
@property (nonatomic, assign) BOOL show_load_more;
@end
