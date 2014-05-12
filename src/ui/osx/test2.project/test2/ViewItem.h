//
//  ViewItem.h
//  kopsik_ui_osx
//
//  Copyright (c) 2014 Toggl Desktop developers. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "kopsik_api.h"

@interface ViewItem : NSObject
+ (NSMutableArray *)loadAll:(KopsikViewItem *)first;
- (void)load:(KopsikViewItem *)data;
@property uint64_t ID;
@property (strong) NSString *GUID;
@property (strong) NSString *Name;
@end


