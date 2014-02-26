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
@property (strong) NSString *Name;
@property unsigned int ID;
@end


