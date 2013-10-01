//
//  BugsnagEvent.m
//  bugsnag
//
//  Created by Simon Maynard on 8/28/13.
//  Copyright (c) 2013 Simon Maynard. All rights reserved.
//

#include <dlfcn.h>
#import <execinfo.h>
#import <mach-o/dyld.h>
#import <mach-o/arch.h>

#import "BugsnagEvent.h"

@interface BugsnagEvent ()
@property (atomic, strong) NSMutableDictionary *dictionary;

- (void) addExceptionWithClass:(NSString*) errorClass andMessage:(NSString*) message andStacktrace:(NSArray*) stacktrace;
@end

@implementation BugsnagEvent

- (id) initWithConfiguration:(BugsnagConfiguration *)configuration andMetaData:(NSDictionary*)metaData {
    if (self = [super init]) {
        self.dictionary = [[NSMutableDictionary alloc] init];
        
        if (configuration.userId != nil) self.userId = configuration.userId;
        if (configuration.appVersion != nil) self.appVersion = configuration.appVersion;
        if (configuration.osVersion != nil) self.osVersion = configuration.osVersion;
        if (configuration.context != nil) self.context = configuration.context;
        if (configuration.releaseStage != nil) self.releaseStage = configuration.releaseStage;
        
        if (configuration.metaData != nil) {
            self.metaData = [configuration.metaData mutableCopy];
        } else {
            self.metaData = [[BugsnagMetaData alloc] init];
        }
        
        if (metaData != nil) [self.metaData mergeWith:metaData];
    }
    return self;
}

- (void) addSignal:(int) signal {
    NSString *errorClass = [NSString stringWithCString:strsignal(signal) encoding:NSUTF8StringEncoding];
    NSArray *stacktrace = [self getStackTraceWithException:nil];
    
    [self addExceptionWithClass:errorClass andMessage:nil andStacktrace:stacktrace];
}

- (void) addException:(NSException*)exception {
    NSArray *stacktrace = [self getStackTraceWithException:exception];
    //TODO:SM can we use userdata on the exception as metadata? Would require filtering
    
    [self addExceptionWithClass:exception.name andMessage:exception.reason andStacktrace:stacktrace];
}

- (void) addExceptionWithClass:(NSString*) errorClass andMessage:(NSString*) message andStacktrace:(NSArray*) stacktrace {
    @synchronized(self) {
        NSMutableArray *exceptions = [self.dictionary objectForKey:@"exceptions"];
        if (exceptions == nil) {
            exceptions = [NSMutableArray array];
            [self.dictionary setObject:exceptions forKey:@"exceptions"];
        }
        
        NSMutableDictionary *dictionary = [NSMutableDictionary dictionaryWithObjectsAndKeys: errorClass, @"errorClass",
                                                                                             stacktrace, @"stacktrace", nil];
        if (message != nil) [dictionary setObject:message forKey:@"message"];
        
        [exceptions addObject:dictionary];
    }
}

- (NSArray *) getStackTraceWithException:(NSException*) exception {
    int count = 256;
    void *frames[count];
    int offset = 0;
    
    // Try to grab the addresses from the exception, if not just grab what we have now
    if (exception != nil && [[exception callStackReturnAddresses] count] != 0 ) {
        NSArray *stackFrames = [exception callStackReturnAddresses];
        count = (int)stackFrames.count;
        for (int i = 0; i < count; ++i) {
            frames[i] = (void *)[[stackFrames objectAtIndex:i] longValue];
        }
    } else {
        count = backtrace(frames, count);
    }
    Dl_info info;
    
    NSMutableArray *stackTrace = [NSMutableArray array];
    NSDictionary *loadedImages = [self loadedImages];
    
    for(uint64_t i = offset; i < count; i++) {
        uint64_t frameAddress = (uint64_t)frames[i];

        int status = dladdr((void *)frameAddress, &info);
        if (status != 0) {
            NSString *fileName = [NSString stringWithCString:info.dli_fname encoding:NSUTF8StringEncoding];
            NSString *binaryName = [NSString stringWithCString:rindex(info.dli_fname, '/') + sizeof(char) encoding:NSUTF8StringEncoding];
            NSDictionary *image = [loadedImages objectForKey:fileName];
            NSMutableDictionary *frame = [NSMutableDictionary dictionaryWithObjectsAndKeys:
                                          [image objectForKey:@"machoUUID"], @"machoUUID",
                                          binaryName, @"machoFile",
                                          nil];
            
            if ([binaryName isEqualToString:[[NSProcessInfo processInfo] processName]]) [frame setObject:[NSNumber numberWithBool:YES] forKey:@"inProject"];

            [frame setObject:[NSString stringWithFormat:@"0x%llx", frameAddress] forKey:@"frameAddress"];
            
            [frame setObject:[image objectForKey:@"machoVMAddress"] forKey:@"machoVMAddress"];
            [frame setObject:[image objectForKey:@"machoLoadAddress"] forKey:@"machoLoadAddress"];
            
            if (info.dli_saddr) {
                [frame setObject:[NSString stringWithFormat:@"0x%llx", (uint64_t)info.dli_saddr] forKey:@"symbolAddress"];
            }

            if (info.dli_sname != NULL && strcmp(info.dli_sname, "<redacted>") != 0) {
                NSString *method = [NSString stringWithCString:info.dli_sname encoding:NSUTF8StringEncoding];
                [frame setObject:method forKey:@"method"];
            }
            
            [stackTrace addObject:[NSDictionary dictionaryWithDictionary:frame]];
        }
    }
    
    return [NSArray arrayWithArray:stackTrace];
}

- (NSDictionary *) loadedImages {
    //Get count of all currently loaded images
    uint32_t count = _dyld_image_count();
    NSMutableDictionary *returnValue = [NSMutableDictionary dictionary];
    
    for (uint32_t i = 0; i < count; i++) {
        const char *dyld = _dyld_get_image_name(i);
        const struct mach_header *header32 = _dyld_get_image_header(i);
        const struct mach_header_64 *header64 = (struct mach_header_64 *)_dyld_get_image_header(i);
        BOOL machHeader64Bit = header32->magic == MH_MAGIC_64;
        
        NSString *machoFile = [NSString stringWithCString:dyld encoding:NSUTF8StringEncoding];
        NSString *machoLoadAddress = [NSString stringWithFormat:@"0x%llx", (uint64_t)header32];
        NSString *machoUUID = nil;
        NSString *machoVMAddress = nil;
        
        // Now lets look at the load_commands
        uint8_t *header_ptr = (uint8_t*)header32;
        header_ptr += machHeader64Bit ? sizeof(struct mach_header_64) : sizeof(struct mach_header);
        struct load_command *command = (struct load_command*)header_ptr;
        
        int numCommands = machHeader64Bit ? header64->ncmds : header32->ncmds;
        
        for (int i = 0; i < numCommands > 0; ++i) {
            if (command->cmd == LC_UUID) {
                struct uuid_command ucmd = *(struct uuid_command*)header_ptr;
                
                CFUUIDRef cuuid = CFUUIDCreateFromUUIDBytes(kCFAllocatorDefault, *((CFUUIDBytes*)ucmd.uuid));
                CFStringRef suuid = CFUUIDCreateString(kCFAllocatorDefault, cuuid);
                CFStringEncoding encoding = CFStringGetFastestEncoding(suuid);
                
                machoUUID = [NSString stringWithCString:CFStringGetCStringPtr(suuid, encoding) encoding:NSUTF8StringEncoding];
                
                CFRelease(cuuid);
                CFRelease(suuid);
            } else if (command->cmd == LC_SEGMENT) {
                struct segment_command ucmd32 = *(struct segment_command*)header_ptr;
                
                if ( strcmp("__TEXT", ucmd32.segname) == 0) {
                    machoVMAddress = [NSString stringWithFormat:@"0x%x", (uint32_t)ucmd32.vmaddr];
                }
            } else if (command->cmd == LC_SEGMENT_64) {
                struct segment_command_64 ucmd64 = *(struct segment_command_64*)header_ptr;
                
                if ( strcmp("__TEXT", ucmd64.segname) == 0) {
                    if (machHeader64Bit) {
                        machoVMAddress = [NSString stringWithFormat:@"0x%llx", (uint64_t)ucmd64.vmaddr];
                    }
                }
            }
            
            header_ptr += command->cmdsize;
            command = (struct load_command*)header_ptr;
        }
        
        NSDictionary *objectValues = [NSDictionary dictionaryWithObjectsAndKeys:machoFile, @"machoFile",
                                                                                machoLoadAddress, @"machoLoadAddress",
                                                                                machoUUID, @"machoUUID",
                                                                                machoVMAddress, @"machoVMAddress", nil];
        [returnValue setObject:objectValues forKey:machoFile];
    }
    return returnValue;
}

- (NSDictionary *) toDictionary {
    @synchronized(self) {
        [self.dictionary setObject:[self.metaData toDictionary] forKey:@"metaData"];
        return [NSDictionary dictionaryWithDictionary:self.dictionary];
    }
}

- (void) setUserAttribute:(NSString*)attributeName withValue:(id)value {
    [self addAttribute:attributeName withValue:value toTabWithName:USER_TAB_NAME];
}

- (void) clearUser {
    [self.metaData clearTab:USER_TAB_NAME];
}

- (void) addAttribute:(NSString*)attributeName withValue:(id)value toTabWithName:(NSString*)tabName {
    [self.metaData addAttribute:attributeName withValue:value toTabWithName:tabName];
}

- (void) clearTabWithName:(NSString*)tabName {
    [self.metaData clearTab:tabName];
}

- (NSString*) appVersion {
    @synchronized(self) {
        return [self.dictionary objectForKey:@"appVersion"];
    }
}

- (void) setAppVersion:(NSString *)appVersion {
    @synchronized(self) {
        [self.dictionary setObject:appVersion forKey:@"appVersion"];
    }
}

- (NSString*) osVersion {
    @synchronized(self) {
        return [self.dictionary objectForKey:@"osVersion"];
    }
}

- (void) setOsVersion:(NSString *)osVersion {
    @synchronized(self) {
        [self.dictionary setObject:osVersion forKey:@"osVersion"];
    }
}

- (NSString*) context {
    @synchronized(self) {
        return [self.dictionary objectForKey:@"context"];
    }
}

- (void) setContext:(NSString *)context {
    @synchronized(self) {
        [self.dictionary setObject:context forKey:@"context"];
    }
}

- (NSString*) releaseStage {
    @synchronized(self) {
        return [self.dictionary objectForKey:@"releaseStage"];
    }
}

- (void) setReleaseStage:(NSString *)releaseStage {
    @synchronized(self) {
        [self.dictionary setObject:releaseStage forKey:@"releaseStage"];
    }
}

- (NSString*) userId {
    @synchronized(self) {
        return [self.dictionary objectForKey:@"userId"];
    }
}

- (void) setUserId:(NSString *)userId {
    @synchronized(self) {
        [self.dictionary setObject:userId forKey:@"userId"];
    }
}
@end
