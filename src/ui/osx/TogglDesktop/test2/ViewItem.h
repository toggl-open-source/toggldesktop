//
//  ViewItem.h
//  Toggl Desktop on the Mac
//
//  Copyright (c) 2014 Toggl Desktop developers. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "kopsik_api.h"

@interface ViewItem : NSObject
+ (NSMutableArray *)loadAll:(KopsikViewItem *)first;
- (void)load:(KopsikViewItem *)data;
@property uint64_t ID;
@property uint64_t WID;
@property (strong) NSString *GUID;
@property (strong) NSString *Name;
@end


