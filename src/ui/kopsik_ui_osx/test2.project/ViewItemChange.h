//
//  ViewItemChange.h
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 10/10/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "kopsik_api.h"

@interface ViewItemChange : NSObject
- (void)load:(KopsikViewItemChange *)data;
@property unsigned int model_type;
@property unsigned int change_type;
@property unsigned int model_id;
@property NSString *GUID;
@end
