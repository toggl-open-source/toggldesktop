//
//  IdleEvent.h
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 04/11/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface IdleEvent : NSObject
@property NSDate *started;
@property NSDate *finished;
@property int seconds;
@end
