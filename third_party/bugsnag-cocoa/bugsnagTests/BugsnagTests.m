//
//  BugsnagTests.m
//  BugsnagTests
//
//  Created by Simon Maynard on 8/28/13.
//  Copyright (c) 2013 Simon Maynard. All rights reserved.
//

#import "BugsnagTests.h"
#import "BugsnagEvent.h"
#import "BugsnagNotifier.h"

static NSData *g_payload;
static NSURL *g_url;

@interface TestBugsnagNotifier : BugsnagNotifier
@end
@implementation TestBugsnagNotifier

- (BOOL) transmitPayload:(NSData *)payload toURL:(NSURL*)url {
    g_url = url;
    g_payload = payload;
    return TRUE;
}
@end

@implementation BugsnagTests

- (void)setUp
{
    [super setUp];
    // Set-up code here.
}

- (void)tearDown
{
    // Tear-down code here.
    [super tearDown];
}

- (void)testExceptions {
    BugsnagConfiguration *configuration = [[BugsnagConfiguration alloc] init];
    configuration.apiKey = @"12345678901234567890123456789012";
    TestBugsnagNotifier *notifier = [[TestBugsnagNotifier alloc] initWithConfiguration: configuration];
    [notifier notifySignal:7];

    NSError *error;
    NSDictionary *json = [NSJSONSerialization JSONObjectWithData:g_payload options:0 error:&error];
    STAssertNil(error, @"should be valid json");
    NSLog(@"hello world %@", json);

    STAssertEqualObjects([json valueForKey:@"apiKey"], @"12345678901234567890123456789012", @"It should send the correct API key");
    STAssertEqualObjects([g_url host], @"notify.bugsnag.com", @"It should send to the correct URL");
}
@end
