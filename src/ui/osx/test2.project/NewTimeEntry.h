//
//  NewTimeEntry.h
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 28/11/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NewTimeEntry : NSObject
@property (strong) NSString *Description;
@property (strong) NSString *Duration;
@property NSString *ProjectAndTaskLabel;
@property unsigned int TaskID;
@property unsigned int ProjectID;
@end
