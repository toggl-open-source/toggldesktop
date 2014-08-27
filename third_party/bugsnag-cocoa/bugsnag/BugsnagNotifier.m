//
//  BugsnagNotifier.m
//  Bugsnag
//
//  Created by Simon Maynard on 8/28/13.
//  Copyright (c) 2013 Simon Maynard. All rights reserved.
//

#import <execinfo.h>
#import <sys/sysctl.h>
#import <mach/mach.h>

#ifdef TARGET_IPHONE_SIMULATOR
#if TARGET_IPHONE_SIMULATOR
#import <UIKit/UIKit.h>
#endif
#endif

#import "BugsnagNotifier.h"
#import "BugsnagLogger.h"

@interface BugsnagNotifier ()
- (BOOL) transmitPayload:(NSData *)payload toURL:(NSURL*)url;
- (void) addDiagnosticsToEvent:(BugsnagEvent*)event;

@property (readonly) NSDictionary* memoryStats;
@property (readonly) NSString *model;
@property (readonly) NSString* machine;
@property (readonly) NSString* networkReachability;
@property (readonly) NSString* appVersion;
@property (readonly) NSString* osVersion;
@end

@implementation BugsnagNotifier

- (id) initWithConfiguration:(BugsnagConfiguration*) configuration {
    if((self = [super init])) {
        self.configuration = configuration;
        
        if (self.configuration.appVersion == nil) self.configuration.appVersion = self.appVersion;
        if (self.configuration.osVersion == nil) self.configuration.osVersion = self.osVersion;
        if (self.configuration.userId == nil) self.configuration.userId = self.userUUID;
        
        [self.configuration.metaData addAttribute:@"Machine" withValue:self.machine toTabWithName:@"device"];
        [self.configuration.metaData addAttribute:@"Model" withValue:self.model toTabWithName:@"device"];
        
        [self beforeNotify:^(BugsnagEvent *event) {
            [self addDiagnosticsToEvent:event];
            return YES;
        }];
        
        self.notifierName = @"Bugsnag Objective-C";
#ifdef COCOAPODS_VERSION_MAJOR_Bugsnag
        self.notifierVersion = [NSString stringWithFormat:@"%i.%i.%i", COCOAPODS_VERSION_MAJOR_Bugsnag, COCOAPODS_VERSION_MINOR_Bugsnag, COCOAPODS_VERSION_PATCH_Bugsnag];
#else
        self.notifierVersion = @"3.0.1";
#endif
        self.notifierURL = @"https://github.com/bugsnag/bugsnag-objective-c";
    }
    return self;
}

- (void) start {
    [self performSelectorInBackground:@selector(backgroundStart) withObject:nil];
}

- (void) addDiagnosticsToEvent:(BugsnagEvent*)event {
    [event addAttribute:@"Application Version" withValue:self.configuration.appVersion toTabWithName:@"application"];
    
    [event addAttribute:@"OS Version" withValue:self.configuration.osVersion toTabWithName:@"device"];
/*
    Do not use network reachability. Since we're reachability ourselves, it will blow up.
    [event addAttribute:@"Network" withValue:self.networkReachability toTabWithName:@"device"];
*/
    
#ifdef TARGET_IPHONE_SIMULATOR
#if !TARGET_IPHONE_SIMULATOR
    [event addAttribute:@"Memory" withValue:self.memoryStats toTabWithName:@"device"];
#endif
#else
    [event addAttribute:@"Memory" withValue:self.memoryStats toTabWithName:@"device"];
#endif
}

- (void) notifySignal:(int)signal {
    if([self shouldAutoNotify]) {
        BugsnagEvent *event = [[BugsnagEvent alloc] initWithConfiguration:self.configuration andMetaData:nil];
        [event addSignal:signal];
        [self notifyEvent:event inBackground: false];
    }
}

- (void) notifyUncaughtException:(NSException *)exception {
    if ([self shouldAutoNotify]) {
        [self notifyException:exception withData:nil inBackground:false];
    }
}

- (void) notifyException:(NSException*)exception withData:(NSDictionary*)metaData inBackground:(BOOL)inBackground {
    if ([self shouldNotify]) {
        BugsnagEvent *event = [[BugsnagEvent alloc] initWithConfiguration:self.configuration andMetaData:metaData];
        [event addException:exception];
        [self notifyEvent:event inBackground: inBackground];
    }
}

- (BOOL) notifyEvent:(BugsnagEvent*) event inBackground:(BOOL) inBackground{
    @synchronized(self.configuration) {
        for (BugsnagNotifyBlock block in self.configuration.beforeBugsnagNotifyBlocks) {
            BOOL retVal = block(event);
            if (retVal == NO) {
                return NO;
            }
        }
    }
    if (inBackground) {
        [self performSelectorInBackground:@selector(backgroundNotifyEvent:) withObject:event];
    } else {
        [self saveEvent:event];
        [self sendSavedEvents];
    }
    return YES;
}

- (void) backgroundNotifyEvent: (BugsnagEvent*)event {
    @autoreleasepool {
        [self saveEvent:event];
        [self sendSavedEvents];
    }
}

- (void) backgroundStart {
    @autoreleasepool {
        [self sendMetrics];
        [self sendSavedEvents];
    }
}

- (BOOL) shouldNotify {
    return self.configuration.notifyReleaseStages == nil || [self.configuration.notifyReleaseStages containsObject:self.configuration.releaseStage];
}

- (BOOL) shouldAutoNotify {
    return self.configuration.autoNotify && [self shouldNotify];
}

- (NSDictionary*) buildNotifyPayload {
    NSDictionary *notifierDetails = [NSDictionary dictionaryWithObjectsAndKeys:self.notifierName, @"name",
                                                                               self.notifierVersion, @"version",
                                                                               self.notifierURL, @"url", nil];
    NSDictionary *payload = [NSDictionary dictionaryWithObjectsAndKeys:notifierDetails, @"notifier",
                                                                       self.configuration.apiKey, @"apiKey",
                                                                       [NSMutableArray array], @"events", nil];
    
    return payload;
}

- (void) beforeNotify:(BugsnagNotifyBlock)block {
    @synchronized(self.configuration) {
        [self.configuration.beforeBugsnagNotifyBlocks addObject:[block copy]];
    }
}

- (void) saveEvent:(BugsnagEvent*)event {
    [[NSFileManager defaultManager] createDirectoryAtPath:self.errorPath withIntermediateDirectories:YES attributes:nil error:nil];
    
    NSDictionary *eventDictionary = [event toDictionary];
    
    if(![eventDictionary writeToFile:[self generateEventFilename] atomically:YES]) {
        BugsnagLog(@"BUGSNAG: Unable to write event file!");
    }
}

- (NSArray *) savedEvents {
	NSArray *directoryContents = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:self.errorPath error:nil];
	NSMutableArray *savedReports = [NSMutableArray arrayWithCapacity:[directoryContents count]];
	for (NSString *file in directoryContents) {
		if ([[file pathExtension] isEqualToString:@"bugsnag"]) {
			NSString *crashPath = [self.errorPath stringByAppendingPathComponent:file];
			[savedReports addObject:crashPath];
		}
	}
	return savedReports;
}

- (void) sendSavedEvents {
    @synchronized(self) {
        @try {
            NSArray *savedEvents = [self savedEvents];
            for ( NSString *file in savedEvents ) {
                NSMutableDictionary *event = [NSMutableDictionary dictionaryWithContentsOfFile:file];
                if (event == nil || [self sendEvent:event]) {
                    [[NSFileManager defaultManager] removeItemAtPath:file error:nil];
                }
            }
        }
        @catch (NSException *exception) {
            BugsnagLog(@"Exception while sending bugsnag events: %@", exception);
        }
    }
}

- (BOOL) sendEvent:(NSDictionary*)event {
    if (event == nil) {
        return NO;
    }
    
    NSDictionary *notifyPayload = [self buildNotifyPayload];
    [[notifyPayload objectForKey:@"events"] addObject:event];
    
    NSData *jsonPayload = [NSJSONSerialization dataWithJSONObject:notifyPayload options:0 error:nil];
    
    return [self transmitPayload:jsonPayload toURL:self.configuration.notifyURL];
}

- (BOOL) sendMetrics {
    NSMutableDictionary *payload = [NSMutableDictionary dictionary];
    [payload setObject:self.configuration.apiKey forKey:@"apiKey"];
    [payload setObject:self.userUUID forKey:@"userId"];
    [payload setObject:self.machine forKey:@"machine"];
    [payload setObject:self.model forKey:@"model"];
    if (self.configuration.osVersion != nil ) [payload setObject:self.configuration.osVersion forKey:@"osVersion"];
    if (self.configuration.appVersion != nil ) [payload setObject:self.configuration.appVersion forKey:@"appVersion"];
    
    NSData *jsonPayload = [NSJSONSerialization dataWithJSONObject:payload options:0 error:nil];
    
    return [self transmitPayload:jsonPayload toURL:self.configuration.metricsURL];
}

- (NSString*) errorPath {
    @synchronized(_errorPath) {
        if(_errorPath) return _errorPath;
        NSArray *folders = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
        NSString *filename = [folders count] == 0 ? NSTemporaryDirectory() : [folders objectAtIndex:0];
        _errorPath = [filename stringByAppendingPathComponent:@"bugsnag"];
        return _errorPath;
    }
}

- (NSString *) generateEventFilename {
    return [[self.errorPath stringByAppendingPathComponent:[[NSProcessInfo processInfo] globallyUniqueString]] stringByAppendingPathExtension:@"bugsnag"];
}

- (BOOL) transmitPayload:(NSData *)payload toURL:(NSURL*)url {
    if(payload){
        NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:url];
        
        [request setHTTPMethod:@"POST"];
        [request setHTTPBody:payload];
        [request setValue:@"application/json" forHTTPHeaderField:@"content-type"];
        
        NSURLResponse* response = nil;
        [NSURLConnection sendSynchronousRequest:request returningResponse:&response error:nil];
        
        NSInteger statusCode = [((NSHTTPURLResponse *)response) statusCode];
        if (statusCode != 200) {
            BugsnagLog(@"Bad response from bugsnag received: %ld.", (long)statusCode);
        } else {
            return YES;
        }
    }
    return NO;
}

- (NSString *) userUUID {
    @synchronized(_uuid) {
        // Return the already determined the UUID
        if(_uuid) return _uuid;
        
        // Try to read UUID from NSUserDefaults
        _uuid = [[NSUserDefaults standardUserDefaults] stringForKey:self.configuration.uuidPath];
        if(_uuid) return _uuid;
        
        // Generate a fresh UUID
        CFUUIDRef uuid = CFUUIDCreate(kCFAllocatorDefault);
        _uuid = (NSString *)CFBridgingRelease(CFUUIDCreateString(kCFAllocatorDefault, uuid));
        CFRelease(uuid);
        
        // Try to save the UUID to the NSUserDefaults
        NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
        [defaults setValue:_uuid forKey:self.configuration.uuidPath];
        [defaults synchronize];
        return _uuid;
    }
}

- (NSString *) machine {
    size_t size = 256;
	char *machineCString = malloc(size);
    sysctlbyname("hw.machine", machineCString, &size, NULL, 0);
    NSString *machine = [NSString stringWithCString:machineCString encoding:NSUTF8StringEncoding];
    free(machineCString);
    
    return machine;
}

- (NSString *) model {
    size_t size = 256;
	char *modelCString = malloc(size);
    sysctlbyname("hw.model", modelCString, &size, NULL, 0);
    NSString *model = [NSString stringWithCString:modelCString encoding:NSUTF8StringEncoding];
    free(modelCString);
    
    return model;
}

- (NSString *)fileSize:(NSNumber *)value {
    float fileSize = [value floatValue];
    if (fileSize<1023.0f)
        return([NSString stringWithFormat:@"%i bytes",[value intValue]]);
    fileSize = fileSize / 1024.0f;
    if (fileSize<1023.0f)
        return([NSString stringWithFormat:@"%1.1f KB",fileSize]);
    fileSize = fileSize / 1024.0f;
    if (fileSize<1023.0f)
        return([NSString stringWithFormat:@"%1.1f MB",fileSize]);
    fileSize = fileSize / 1024.0f;
    
    return([NSString stringWithFormat:@"%1.1f GB",fileSize]);
}

- (NSString *) networkReachability {
    Class reachabilityClass = NSClassFromString(@"Reachability");
    if (reachabilityClass == nil) reachabilityClass = NSClassFromString(@"BugsnagReachability");
    if (reachabilityClass == nil) return nil;
    
    id reachability = [reachabilityClass performSelector:@selector(reachabilityForInternetConnection)];
    [reachability performSelector:@selector(startNotifier)];
    NSString *returnValue = [reachability performSelector:@selector(currentReachabilityString)];
    [reachability performSelector:@selector(stopNotifier)];
    
    return returnValue;
}

- (NSDictionary *) memoryStats {
    NSMutableDictionary *memoryStats = [NSMutableDictionary dictionary];
    
    struct task_basic_info info;
    mach_msg_type_number_t size = sizeof(info);
    kern_return_t kerr = task_info(mach_task_self(),
                                   TASK_BASIC_INFO,
                                   (task_info_t)&info,
                                   &size);
    if( kerr == KERN_SUCCESS ) {
        [memoryStats setObject:[self fileSize:[NSNumber numberWithInteger:info.resident_size]] forKey:@"App Using"];
    }
    
    uint64_t total = 0;
    uint64_t pageSize = 0;
    uint64_t pagesFree = 0;
    size_t sysCtlSize = sizeof(uint64_t);
    if (!sysctlbyname("hw.memsize", &total, &sysCtlSize, NULL, 0)) {
        [memoryStats setObject:[self fileSize:[NSNumber numberWithInteger:total]] forKey:@"Total"];
    }
    
    if (!sysctlbyname("vm.page_free_count", &pagesFree, &sysCtlSize, NULL, 0)) {
        if (!sysctlbyname("hw.pagesize", &pageSize, &sysCtlSize, NULL, 0)) {
            [memoryStats setObject:[self fileSize:[NSNumber numberWithInteger:pagesFree*pageSize]] forKey:@"Free"];
            [memoryStats setObject:[self fileSize:[NSNumber numberWithInteger:total-(pagesFree*pageSize)]] forKey:@"Used"];
        }
    }
    
    return memoryStats;
}

- (NSString *) appVersion {
    NSString *bundleVersion = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleVersion"];
    NSString *versionString = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleShortVersionString"];
    if (bundleVersion != nil && versionString != nil && ![bundleVersion isEqualToString:versionString]) {
        return [NSString stringWithFormat:@"%@ (%@)", versionString, bundleVersion];
    } else if (bundleVersion != nil) {
        return bundleVersion;
    } else if(versionString != nil) {
        return versionString;
    }
    return @"";
}

- (NSString *) osVersion {
#ifdef TARGET_IPHONE_SIMULATOR
#if TARGET_IPHONE_SIMULATOR
	return [[UIDevice currentDevice] systemVersion];
#else
    return [[NSProcessInfo processInfo] operatingSystemVersionString];
#endif
#else
	return [[NSProcessInfo processInfo] operatingSystemVersionString];
#endif
}

@end
