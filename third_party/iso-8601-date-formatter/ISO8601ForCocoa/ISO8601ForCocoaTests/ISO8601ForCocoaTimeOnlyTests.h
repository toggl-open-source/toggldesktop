//
//  ISO8601ForCocoaTimeOnlyTests.h
//  ISO8601ForCocoa
//
//  Created by Peter Hosey on 2013-09-17.
//  Copyright (c) 2013 Peter Hosey. All rights reserved.
//

#import <SenTestingKit/SenTestingKit.h>

@interface ISO8601ForCocoaTimeOnlyTests : SenTestCase

- (void) testParsingStringWithOnlyHourMinuteSecondZulu;
- (void) testParsingStringWithOnlyHourMinuteZulu;
- (void) testParsingStringWithOnlyHourMinuteSecondAndTimeZone;
- (void) testParsingStringWithOnlyHourMinuteAndTimeZone;

@end
