//
//  BugsnagMetadata.h
//  bugsnag
//
//  Created by Simon Maynard on 8/28/13.
//  Copyright (c) 2013 Simon Maynard. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface BugsnagMetaData : NSObject < NSMutableCopying >

- (id) initWithDictionary:(NSMutableDictionary*)dict;
- (NSMutableDictionary *) getTab:(NSString*)tabName;
- (void) clearTab:(NSString*)tabName;
- (void) mergeWith:(NSDictionary*)data;
- (NSDictionary*) toDictionary;
- (void) addAttribute:(NSString*)attributeName withValue:(id)value toTabWithName:(NSString*)tabName;

@end
