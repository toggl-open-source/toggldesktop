//
//  BugsnagLogger.h
//  Bugsnag
//
//  Created by Simon Maynard on 8/28/13.
//  Copyright (c) 2013 Simon Maynard. All rights reserved.
//

#import <Foundation/Foundation.h>

#ifdef DEBUG
#   define BugsnagLog(__FORMAT__, ...) NSLog(__FORMAT__, ##__VA_ARGS__)
#else
#   define BugsnagLog(...) do {} while (0)
#endif
