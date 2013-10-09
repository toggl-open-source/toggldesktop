//
//  ISO8601ForCocoaCalendarDateTests.h
//  ISO8601ForCocoaCalendarDateTests
//
//  Created by Peter Hosey on 2013-05-27.
//  Copyright (c) 2013 Peter Hosey. All rights reserved.
//

#import <SenTestingKit/SenTestingKit.h>

//This should be set to 1 (actually, removed from all uses) after the dateComponentsFromString: methods are refactored to behave reasonably on their own, rather than as the implementation of the dateFromString: methods.
//For example, “T22” currently returns mostly zero components, when it should return all but one undefined. The filling in of zeroes and the current date should happen on dateFromString:'s side. That refactor is on hold pending much more test coverage (especially of dateFromString: cases).
#define POST_DATE_COMPONENTS_REFACTOR 0

@interface ISO8601ForCocoaCalendarDateTests : SenTestCase

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
#if POST_DATE_COMPONENTS_REFACTOR
- (void) testParsingDateWithTimeOnly;
#endif

- (void) testUnparsingDatesWithoutTime;

//Test case for https://github.com/boredzo/iso-8601-date-formatter/issues/6
- (void) testUnparsingDateInDaylightSavingTime;

//Test case for https://github.com/boredzo/iso-8601-date-formatter/issues/3 and https://github.com/boredzo/iso-8601-date-formatter/issues/5
- (void) testUnparsingDateWithinBritishSummerTimeAsUTC;

//Test case for https://github.com/boredzo/iso-8601-date-formatter/pull/20
- (void) testStrictModeRejectsSlashyDates;

- (void) testParseNilIntoDateComponents;
- (void) testParseNilIntoDate;

- (void) testStringFromInapplicableObjectValues;

- (void) testParsingDateWithSpaceInFrontOfItStrictly;
- (void) testParsingDateWithSpaceInFrontOfItNonStrictly;

- (void) testParsingSloppyDatesStrictly;

- (void) testParsingDateFromSubstring;

@end
