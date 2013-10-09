//
//  ISO8601ForCocoaWeekDateTests.m
//  ISO8601ForCocoa
//
//  Created by Peter Hosey on 2013-09-11.
//  Copyright (c) 2013 Peter Hosey. All rights reserved.
//

#import "ISO8601ForCocoaWeekDateTests.h"
#import "ISO8601DateFormatter.h"
#import "NSLocale+UnitTestSwizzling.h"
#import "PRHNamedCharacter.h"

static const NSTimeInterval gSecondsPerHour = 3600.0;

@interface ISO8601ForCocoaWeekDateTests ()

- (void)        attemptToParseString:(NSString *)dateString
expectTimeIntervalSinceReferenceDate:(NSTimeInterval)expectedTimeIntervalSinceReferenceDate
	  expectTimeZoneWithHoursFromGMT:(NSTimeInterval)expectedHoursFromGMT;

@end

@implementation ISO8601ForCocoaWeekDateTests
{
	ISO8601DateFormatter *_iso8601DateFormatter;
}

- (void) setUp {
	[super setUp];

	_iso8601DateFormatter = [[ISO8601DateFormatter alloc] init];
}

- (void) tearDown {
	_iso8601DateFormatter = nil;

	[super tearDown];
}

- (void)        attemptToParseString:(NSString *)dateString
expectTimeIntervalSinceReferenceDate:(NSTimeInterval)expectedTimeIntervalSinceReferenceDate
	  expectTimeZoneWithHoursFromGMT:(NSTimeInterval)expectedHoursFromGMT
{
	const NSTimeInterval expectedSecondsFromGMT = expectedHoursFromGMT * gSecondsPerHour;

	NSTimeZone *timeZone = nil;
	NSDate *date = [_iso8601DateFormatter dateFromString:dateString timeZone:&timeZone];
	STAssertNotNil(date, @"Parsing a valid ISO 8601 calendar date should return an NSDate object");
	STAssertNotNil(timeZone, @"Parsing a valid ISO 8601 calendar date that specifies a time zone offset should return an NSTimeZone object");
	STAssertEqualsWithAccuracy([date timeIntervalSinceReferenceDate], expectedTimeIntervalSinceReferenceDate, 0.0001, @"Date parsed from '%@' (%@) should be %f seconds since the reference date (%@)", dateString, date, expectedTimeIntervalSinceReferenceDate, [NSDate dateWithTimeIntervalSinceReferenceDate:expectedTimeIntervalSinceReferenceDate]);
	NSInteger secondsFromGMTForDate = [timeZone secondsFromGMTForDate:date];
	STAssertEquals(secondsFromGMTForDate, (NSInteger)expectedSecondsFromGMT, @"Time zone parsed from '%@' should be %f seconds (%f hours) from GMT, not %ld seconds (%f hours)", dateString, expectedSecondsFromGMT, expectedHoursFromGMT, secondsFromGMTForDate, secondsFromGMTForDate / gSecondsPerHour);
}

- (void) attemptToUnparseDateWithTimeIntervalSinceReferenceDate:(NSTimeInterval)timeIntervalSinceReferenceDate
                                                   timeZoneName:(NSString *)tzName
			                                   expectDateString:(NSString *)expectedDateString
							                        includeTime:(bool)includeTime
{
	NSDate *date = [NSDate dateWithTimeIntervalSinceReferenceDate:timeIntervalSinceReferenceDate];
	NSTimeZone *timeZone = [NSTimeZone timeZoneWithName:tzName];

	_iso8601DateFormatter.includeTime = includeTime;
	_iso8601DateFormatter.format = ISO8601DateFormatWeek;

	NSString *dateString = [_iso8601DateFormatter stringFromDate:date timeZone:timeZone];
	STAssertNotNil(dateString, @"Unparsing a date should return a string");
	STAssertEqualObjects(dateString, expectedDateString, @"Got unexpected output for date with time interval since reference date %f in time zone %@", timeIntervalSinceReferenceDate, timeZone);
}

- (void) testParsingDateInPacificStandardTime {
	static NSString *const dateString = @"2007-W01-01T13:24:56-0800";
	static NSTimeInterval const expectedTimeIntervalSinceReferenceDate = 189379496.0;
	static NSTimeInterval const expectedHoursFromGMT = -8.0;

	[self attemptToParseString:dateString expectTimeIntervalSinceReferenceDate:expectedTimeIntervalSinceReferenceDate
expectTimeZoneWithHoursFromGMT:expectedHoursFromGMT];
}

- (void) testUnparsingDateInPacificStandardTime {
	NSTimeInterval timeIntervalSinceReferenceDate = 189379496.0;
	NSString *expectedDateString = @"2007-W01-01T13:24:56-0800";
	NSString *tzName = @"America/Los_Angeles";

	[self attemptToUnparseDateWithTimeIntervalSinceReferenceDate:timeIntervalSinceReferenceDate
		timeZoneName:tzName
		expectDateString:expectedDateString
		includeTime:true];
}

- (void) testParsingDateInPacificDaylightTime {
	static NSString *const dateString = @"2007-W31-03T13:24:56-0700";
	static NSTimeInterval const expectedTimeIntervalSinceReferenceDate = 207692696.0;
	static NSTimeInterval const expectedHoursFromGMT = -7.0;

	[self attemptToParseString:dateString expectTimeIntervalSinceReferenceDate:expectedTimeIntervalSinceReferenceDate
expectTimeZoneWithHoursFromGMT:expectedHoursFromGMT];
}

- (void) testUnparsingDateInPacificDaylightTime {
	NSTimeInterval timeIntervalSinceReferenceDate = 207692696.0;
	NSString *expectedDateString = @"2007-W31-03T13:24:56-0700";
	NSString *tzName = @"America/Los_Angeles";

	[self attemptToUnparseDateWithTimeIntervalSinceReferenceDate:timeIntervalSinceReferenceDate
		timeZoneName:tzName
		expectDateString:expectedDateString
		includeTime:true];
}

- (void) testUnparsingDateAtRiskOfAccidentalPM {
    // swizzle [NSLocale currentLocale] with a method that returns a mock object which forces "12 hour mode" on the de_DE locale which naturally uses 24 hour formatting.
    SwizzleClassMethod([NSLocale class], @selector(currentLocale), @selector(mockCurrentLocale));

	_iso8601DateFormatter.includeTime = YES;
	_iso8601DateFormatter.format = ISO8601DateFormatWeek;

	NSTimeInterval timeIntervalSinceReferenceDate = 397143300.0;
	NSDate *date = [NSDate dateWithTimeIntervalSinceReferenceDate:timeIntervalSinceReferenceDate];
	NSTimeZone *tz;

	tz = [NSTimeZone timeZoneWithName:@"GMT"];
	STAssertEqualObjects([_iso8601DateFormatter stringFromDate:date timeZone:tz], @"2013-W31-05T13:35:00Z", @"Unexpected date string for 13:35 on 2 August 2013 in London");

	tz = [NSTimeZone timeZoneWithName:@"Europe/London"];
	STAssertEqualObjects([_iso8601DateFormatter stringFromDate:date timeZone:tz], @"2013-W31-05T14:35:00+0100", @"Unexpected date string for 13:35 on 2 August 2013 in London");

    // swizzle back so only this test is affected
    SwizzleClassMethod([NSLocale class], @selector(currentLocale), @selector(mockCurrentLocale));
}

- (void) testParsingDateInGreenwichMeanTime {
	static NSTimeInterval const expectedTimeIntervalSinceReferenceDate = 189350696.0;
	static NSTimeInterval const expectedHoursFromGMT = -0.0;

	[self attemptToParseString:@"2007-W01-01T13:24:56-0000"
		expectTimeIntervalSinceReferenceDate:expectedTimeIntervalSinceReferenceDate
		expectTimeZoneWithHoursFromGMT:expectedHoursFromGMT];
	[self attemptToParseString:@"2007-W01-01T13:24:56+0000"
		expectTimeIntervalSinceReferenceDate:expectedTimeIntervalSinceReferenceDate
		expectTimeZoneWithHoursFromGMT:expectedHoursFromGMT];
	[self attemptToParseString:@"2007-W01-01T13:24:56Z"
		expectTimeIntervalSinceReferenceDate:expectedTimeIntervalSinceReferenceDate
		expectTimeZoneWithHoursFromGMT:expectedHoursFromGMT];
}

- (void) testUnparsingDateInGreenwichMeanTime {
	NSTimeInterval timeIntervalSinceReferenceDate = 189350696.0;
	NSString *expectedDateString = @"2007-W01-01T13:24:56Z";
	NSString *tzName = @"GMT";

	[self attemptToUnparseDateWithTimeIntervalSinceReferenceDate:timeIntervalSinceReferenceDate
		timeZoneName:tzName
		expectDateString:expectedDateString
		includeTime:true];
}

- (void) testParsingDateWithFractionOfSecondWithoutLosingPrecision {
  NSDate *referenceDate = [_iso8601DateFormatter dateFromString:@"2007-W01-01T13:24:56-0000"];
  NSDate *referenceDateWithAddedMilliseconds = [_iso8601DateFormatter dateFromString:@"2007-W01-01T13:24:56.123-0000"];

  NSTimeInterval differenceBetweenDates = [referenceDateWithAddedMilliseconds timeIntervalSinceDate:referenceDate];

  STAssertEqualsWithAccuracy(differenceBetweenDates, 0.123, 1e-3, @"Expected parsed dates to reflect difference in milliseconds");
}

- (void) testParsingDateWithUnusualTimeSeparator {
	_iso8601DateFormatter.parsesStrictly = false;
	_iso8601DateFormatter.timeSeparator = SNOWMAN;

	static NSString *const dateString = @"2007-W01-01T13☃24☃56-0800";
	static NSTimeInterval const expectedTimeIntervalSinceReferenceDate = 189379496.0;
	static NSTimeInterval const expectedHoursFromGMT = -8.0;

	[self attemptToParseString:dateString expectTimeIntervalSinceReferenceDate:expectedTimeIntervalSinceReferenceDate
expectTimeZoneWithHoursFromGMT:expectedHoursFromGMT];
}

- (void) testUnparsingDateWithUnusualTimeSeparator {
	_iso8601DateFormatter.timeSeparator = SNOWMAN;

	NSTimeInterval timeIntervalSinceReferenceDate = 189379496.0;
	NSString *expectedDateString = @"2007-W01-01T13☃24☃56-0800";
	NSString *tzName = @"America/Los_Angeles";

	[self attemptToUnparseDateWithTimeIntervalSinceReferenceDate:timeIntervalSinceReferenceDate
	                                                timeZoneName:tzName
				                                expectDateString:expectedDateString
								                     includeTime:true];
}

- (void) testParsingDateWithTimeZoneSeparator {
	_iso8601DateFormatter.timeZoneSeparator = SNOWMAN;

	static NSString *const dateString = @"2007-W01-01T13:24:56-07☃30";
	static NSTimeInterval const expectedTimeIntervalSinceReferenceDate = 189377696.0;
	static NSTimeInterval const expectedHoursFromGMT = -7.5;

	[self attemptToParseString:dateString expectTimeIntervalSinceReferenceDate:expectedTimeIntervalSinceReferenceDate
expectTimeZoneWithHoursFromGMT:expectedHoursFromGMT];
}

- (void) testUnparsingDateWithTimeZoneSeparator {
	_iso8601DateFormatter.timeZoneSeparator = ':';

	NSTimeInterval timeIntervalSinceReferenceDate = 189379496.0;
	NSString *expectedDateString = @"2007-W01-01T13:24:56-08:00";
	NSString *tzName = @"America/Los_Angeles";

	[self attemptToUnparseDateWithTimeIntervalSinceReferenceDate:timeIntervalSinceReferenceDate
	                                                timeZoneName:tzName
				                                expectDateString:expectedDateString
								                     includeTime:true];
}

- (void) testParsingDateWithIncompleteTime {
	NSString *string;
	NSTimeInterval expectedTimeIntervalSinceReferenceDate;
	NSDate *date;
	NSDate *expectedDate;

	string = @"2007-W01-01T13:24:56Z";
	expectedTimeIntervalSinceReferenceDate = 189350696.0;
	expectedDate = [NSDate dateWithTimeIntervalSinceReferenceDate:expectedTimeIntervalSinceReferenceDate];
	date = [_iso8601DateFormatter dateFromString:string];
	STAssertEqualObjects(date, expectedDate, @"Parsing string %@ (expected %f); got date %@ (%f)", string, expectedTimeIntervalSinceReferenceDate, date, date.timeIntervalSinceReferenceDate);

	string = @"2007-W01-01T13:24Z";
	expectedTimeIntervalSinceReferenceDate = 189350640.0;
	expectedDate = [NSDate dateWithTimeIntervalSinceReferenceDate:expectedTimeIntervalSinceReferenceDate];
	date = [_iso8601DateFormatter dateFromString:string];
	STAssertEqualObjects(date, expectedDate, @"Parsing string %@ (expected %f); got date %@ (%f)", string, expectedTimeIntervalSinceReferenceDate, date, date.timeIntervalSinceReferenceDate);

	string = @"2007-W01-01T13Z";
	expectedTimeIntervalSinceReferenceDate = 189349200.0;
	expectedDate = [NSDate dateWithTimeIntervalSinceReferenceDate:expectedTimeIntervalSinceReferenceDate];
	date = [_iso8601DateFormatter dateFromString:string];
	STAssertEqualObjects(date, expectedDate, @"Parsing string %@ (expected %f); got date %@ (%f)", string, expectedTimeIntervalSinceReferenceDate, date, date.timeIntervalSinceReferenceDate);
}

- (void) testUnparsingDateWithoutTime {
	_iso8601DateFormatter.format = ISO8601DateFormatWeek;
	_iso8601DateFormatter.includeTime = false;
	_iso8601DateFormatter.defaultTimeZone = [NSTimeZone timeZoneWithName:@"UTC"];

	NSString *expectedString = @"2007-W01-01";
	NSTimeInterval timeIntervalSinceReferenceDate = 189302400.0;
	NSDate *date = [NSDate dateWithTimeIntervalSinceReferenceDate:timeIntervalSinceReferenceDate];
	NSString *string = [_iso8601DateFormatter stringFromDate:date];
	STAssertEqualObjects(string, expectedString, @"Generated wrong week date string for %@ (%f)", date, timeIntervalSinceReferenceDate);
}

- (void) testUnparsingDateInDaylightSavingTime {
	_iso8601DateFormatter.defaultTimeZone = [NSTimeZone timeZoneWithName:@"Europe/Prague"];
	_iso8601DateFormatter.includeTime = YES;
	_iso8601DateFormatter.format = ISO8601DateFormatWeek;

	NSDate *date;
	NSString *string;
	NSString *expectedString;

	date = [NSDate dateWithTimeIntervalSinceReferenceDate:365464800.0];
	string = [_iso8601DateFormatter stringFromDate:date];
	expectedString = @"2012-W31-03T00:00:00+0200";
	STAssertEqualObjects(string, expectedString, @"Got wrong string for first date in DST in Prague #1 (check whether DST is included in TZ offset)");

	date = [NSDate dateWithTimeIntervalSinceReferenceDate:373417200.0];
	string = [_iso8601DateFormatter stringFromDate:date];
	expectedString = @"2012-W44-04T00:00:00+0100";
	STAssertEqualObjects(string, expectedString, @"Got wrong string for second date in DST in Prague #1 (check whether DST is included in TZ offset)");
}

- (void) testUnparsingDateWithinBritishSummerTimeAsUTC {
	_iso8601DateFormatter.includeTime = YES;
	_iso8601DateFormatter.format = ISO8601DateFormatWeek;

	NSDate *date;
	NSString *expectedString;
	NSString *string;
	NSTimeZone *UTCTimeZone = [NSTimeZone timeZoneWithName:@"UTC"];

	date = [NSDate dateWithTimeIntervalSinceReferenceDate:354987473.0];
	expectedString = @"2012-W13-07T15:37:53Z";

	string = [_iso8601DateFormatter stringFromDate:date timeZone:UTCTimeZone];
	STAssertEqualObjects(string, expectedString, @"Got wrong string for April date in UTC (check whether DST is included in TZ offset)");

	_iso8601DateFormatter.defaultTimeZone = UTCTimeZone;
	string = [_iso8601DateFormatter stringFromDate:date];
	STAssertEqualObjects(string, expectedString, @"Got wrong string for April date in UTC-as-default (check whether DST is included in TZ offset)");

	//Date https://github.com/boredzo/iso-8601-date-formatter/issues/3 was filed.
	date = [NSDate dateWithTimeIntervalSinceReferenceDate:370245466.0];
	expectedString = @"2012-W39-02T05:57:46Z";

	string = [_iso8601DateFormatter stringFromDate:date];
	STAssertEqualObjects(string, expectedString, @"Got wrong string for September date in UTC-as-default (check whether DST is included in TZ offset)");
}

@end
