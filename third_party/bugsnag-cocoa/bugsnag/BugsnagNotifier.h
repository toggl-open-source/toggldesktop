//
//  BugsnagNotifier.h
//  Bugsnag
//
//  Created by Simon Maynard on 8/28/13.
//  Copyright (c) 2013 Simon Maynard. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "BugsnagConfiguration.h"
#import "BugsnagEvent.h"

typedef BOOL (^BugsnagNotifyBlock)(BugsnagEvent*);

@interface BugsnagNotifier : NSObject {
    @private
    NSString *_errorPath;
    NSString *_uuid;
}

- (id) initWithConfiguration:(BugsnagConfiguration*) configuration;
- (void) start;

- (void) notifySignal:(int)signal;
- (void) notifyUncaughtException:(NSException*)exception;
- (void) notifyException:(NSException*)exception withData:(NSDictionary*)metaData inBackground:(BOOL)inBackground;

- (BOOL) shouldAutoNotify;
- (BOOL) shouldNotify;

- (NSDictionary*) buildNotifyPayload;
- (void) beforeNotify:(BugsnagNotifyBlock)block;

- (void) saveEvent:(BugsnagEvent*)event;
- (NSString*) generateEventFilename;
- (NSArray*) savedEvents;
- (void) sendSavedEvents;

- (NSString *)fileSize:(NSNumber *)value;

@property (atomic, copy) NSString *notifierName;
@property (atomic, copy) NSString *notifierVersion;
@property (atomic, copy) NSString *notifierURL;
@property (atomic, strong) BugsnagConfiguration *configuration;
@property (atomic, readonly) NSString *errorPath;
@property (atomic, readonly) NSString *userUUID;

@end
