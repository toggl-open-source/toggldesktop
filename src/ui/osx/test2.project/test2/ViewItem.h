//
//  ViewItem.h
//  kopsik_ui_osx
//
//  Copyright (c) 2014 Toggl Desktop developers. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "kopsik_api.h"

@interface ViewItem : NSObject
- (void)load:(KopsikViewItem *)data;
@property unsigned int ID;
@property (strong) NSString *GUID;
@property (strong) NSString *Name;
@end


