//
//  ModelChange.h
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 21/10/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "kopsik_api.h"

@interface ModelChange : NSObject
- (void)load:(KopsikModelChange *)change;
@property (strong) NSString *GUID;
@property (strong) NSString *ModelType;
@property (strong) NSString *ChangeType;
@property unsigned int ModelID;
@end
