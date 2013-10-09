//
//  ISO8601ForCocoaWeekDateTests.h
//  ISO8601ForCocoa
//
//  Created by Peter Hosey on 2013-09-11.
//  Copyright (c) 2013 Peter Hosey. All rights reserved.
//

#import <SenTestingKit/SenTestingKit.h>

@interface ISO8601ForCocoaWeekDateTests : SenTestCase

- (void) testParsingDateInPacificStandardTime;
- (void) testUnparsingDateInPacificStandardTime;

- (void) testParsingDateInPacificDaylightTime;
- (void) testUnparsingDateInPacificDaylightTime;

//Test case for https://github.com/boredzo/iso-8601-date-formatter/issues/15
- (void) testUnparsingDateAtRiskOfAccidentalPM;

- (void) testParsingDateInGreenwichMeanTime;
- (void) testUnparsingDateInGreenwichMeanTime;

- (void) testParsingDateWithFractionOfSecondWithoutLosingPrecision;

- (void) testParsingDateWithUnusualTimeSeparator;
- (void) testUnparsingDateWithUnusualTimeSeparator;

- (void) testParsingDateWithTimeZoneSeparator;
- (void) testUnparsingDateWithTimeZoneSeparator;

- (void) testParsingDateWithIncompleteTime;

- (void) testUnparsingDateWithoutTime;

//Test case for https://github.com/boredzo/iso-8601-date-formatter/issues/6
- (void) testUnparsingDateInDaylightSavingTime;

//Test case for https://github.com/boredzo/iso-8601-date-formatter/issues/3 and https://github.com/boredzo/iso-8601-date-formatter/issues/5
- (void) testUnparsingDateWithinBritishSummerTimeAsUTC;

@end
