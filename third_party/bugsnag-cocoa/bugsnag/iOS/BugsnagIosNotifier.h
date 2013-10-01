//
//  BugsnagIosNotifier.h
//  Bugsnag
//
//  Created by Simon Maynard on 8/28/13.
//  Copyright (c) 2013 Simon Maynard. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "BugsnagNotifier.h"

@interface BugsnagIosNotifier : BugsnagNotifier
- (void) addIosDiagnosticsToEvent:(BugsnagEvent *) event;
@end
