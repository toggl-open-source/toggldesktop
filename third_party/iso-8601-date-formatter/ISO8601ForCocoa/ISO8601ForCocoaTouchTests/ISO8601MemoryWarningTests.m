//
//  ISO8601MemoryWarningTests.m
//  ISO8601ForCocoa
//
//  Created by Peter Hosey on 2013-10-02.
//  Copyright (c) 2013 Peter Hosey. All rights reserved.
//

#import "ISO8601MemoryWarningTests.h"

#import "ISO8601DateFormatter.h"

extern bool ISO8601DateFormatter_GlobalCachesAreWarm(void);

@interface ISO8601DateFormatter (ISO8601MemoryWarningTesting)
+ (void) purgeGlobalCaches;
@end

#import <UIKit/UIKit.h>

@implementation ISO8601MemoryWarningTests
{
	ISO8601DateFormatter *_iso8601DateFormatter;
}

- (void) setUp {
	[super setUp];

	_iso8601DateFormatter = [[ISO8601DateFormatter alloc] init];

	//Just in case this isn't the first test that this process has run…
	[ISO8601DateFormatter purgeGlobalCaches];
}

- (void) tearDown {
	_iso8601DateFormatter = nil;

	[super tearDown];
}

- (void) testMemoryWarning {
	STAssertFalseNoThrow(ISO8601DateFormatter_GlobalCachesAreWarm(), @"Global caches are already warm before using an ISO 8601 date formatter!");

	//Now parse a bunch of dates to try to warm the caches.
	[_iso8601DateFormatter dateFromString:@"2013-09-18T07:34:21-1200"];
	[_iso8601DateFormatter dateFromString:@"2013-09-18T07:34:21-0800"];
	[_iso8601DateFormatter dateFromString:@"2013-09-18T07:34:21-0130"];
	[_iso8601DateFormatter dateFromString:@"2013-09-18T07:34:21-0000"];
	[_iso8601DateFormatter dateFromString:@"2013-09-18T07:34:21+0000"];
	[_iso8601DateFormatter dateFromString:@"2013-09-18T07:34:21+0130"];
	[_iso8601DateFormatter dateFromString:@"2013-09-18T07:34:21+0800"];
	[_iso8601DateFormatter dateFromString:@"2013-09-18T07:34:21+1200"];

	STAssertTrueNoThrow(ISO8601DateFormatter_GlobalCachesAreWarm(), @"Global caches were not warmed by using an ISO 8601 date formatter!");

	[[NSNotificationCenter defaultCenter] postNotificationName:UIApplicationDidReceiveMemoryWarningNotification object:[UIApplication sharedApplication]];

	STAssertFalseNoThrow(ISO8601DateFormatter_GlobalCachesAreWarm(), @"Global caches were not purged by a (simulated) memory warning!");
}

@end
