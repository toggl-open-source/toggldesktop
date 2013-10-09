//
//  ISO8601ForCocoaTimeOnlyTests.m
//  ISO8601ForCocoa
//
//  Created by Peter Hosey on 2013-09-17.
//  Copyright (c) 2013 Peter Hosey. All rights reserved.
//

#import "ISO8601ForCocoaTimeOnlyTests.h"
#import "ISO8601DateFormatter.h"

static const NSTimeInterval gSecondsPerHour = 3600.0;
static const NSTimeInterval gSecondsPerMinute = 60.0;

@implementation ISO8601ForCocoaTimeOnlyTests
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

- (NSString *) dateStringWithHour:(NSTimeInterval)hour
	minute:(NSTimeInterval)minute
	second:(NSTimeInterval)second
	timeZone:(NSTimeZone *)timeZone
{
	NSString *format =
		  second > 0.0 ? @"T%02g:%02g:%02g"
		: minute > 0.0 ? @"T%02g:%02g"
		: hour > 0.0 ? @"T%02g"
		: @"no non-zero components provided!"
	;
	NSString *string = [NSString stringWithFormat:format, hour, minute, second];
	NSInteger secondsFromGMT = [timeZone secondsFromGMT];
	string = secondsFromGMT == 0.0
		? [string stringByAppendingString:@"Z"]
		: [string stringByAppendingFormat:@"%+03g%02g", secondsFromGMT / gSecondsPerHour, fabs(fmod(secondsFromGMT / gSecondsPerMinute, gSecondsPerMinute))];
	return string;
}

- (NSDate *) dateForTodayWithHour:(NSTimeInterval)hour
	minute:(NSTimeInterval)minute
	second:(NSTimeInterval)second
	timeZone:(NSTimeZone *)timeZone
{
	NSDate *now = [NSDate date];
	NSCalendar *calendar = [[NSCalendar alloc] initWithCalendarIdentifier:NSGregorianCalendar];
	calendar.timeZone = timeZone;

	NSDate *today = nil;
	[calendar rangeOfUnit:NSDayCalendarUnit startDate:&today interval:NULL forDate:now];

	NSDateComponents *components = [NSDateComponents new];
	components.hour = (NSInteger)hour;
	components.minute = (NSInteger)minute;
	components.second = (NSInteger)second;

	NSDate *date = [calendar dateByAddingComponents:components toDate:today options:0];
	return date;
}

- (void)        attemptToParseString:(NSString *)dateString
expectTimeIntervalSinceReferenceDate:(NSTimeInterval)expectedTimeIntervalSinceReferenceDate
	  expectTimeZoneWithHoursFromGMT:(NSTimeInterval)expectedHoursFromGMT
{
	const NSTimeInterval expectedSecondsFromGMT = expectedHoursFromGMT * gSecondsPerHour;

	NSTimeZone *timeZone = nil;
	NSDate *date = [_iso8601DateFormatter dateFromString:dateString timeZone:&timeZone];
	STAssertNotNil(date, @"Parsing a valid ISO 8601 date string (%@) should return an NSDate object", dateString);
	STAssertNotNil(timeZone, @"Parsing a valid ISO 8601 date string (%@) that specifies a time zone offset should return an NSTimeZone object", dateString);
	STAssertEqualsWithAccuracy([date timeIntervalSinceReferenceDate], expectedTimeIntervalSinceReferenceDate, 0.0001, @"Date parsed from '%@' should be %f seconds since the reference date (%f seconds difference)", dateString, expectedTimeIntervalSinceReferenceDate, [date timeIntervalSinceDate:[NSDate dateWithTimeIntervalSinceReferenceDate:expectedTimeIntervalSinceReferenceDate]]);
	NSInteger secondsFromGMTForDate = [timeZone secondsFromGMTForDate:date];
	STAssertEquals(secondsFromGMTForDate, (NSInteger)expectedSecondsFromGMT, @"Time zone parsed from '%@' should be %f seconds (%f hours) from GMT, not %ld seconds (%f hours)", dateString, expectedSecondsFromGMT, expectedHoursFromGMT, secondsFromGMTForDate, secondsFromGMTForDate / gSecondsPerHour);
}

/*TODO: These tests are inherently flaky.
 *You can't build a stable test on [NSDate date]—the results will vary according to the current date and are likely to vary by time zone.
 *These tests should probably use some sort of “default date” property of the date formatter, and the date formatter should fill in from the current date if and only if its default date is not set.
 *Additionally, the behavior we're testing is probably best modeled by dateComponentsFromString::, not dateFromString::, once dCFS:: is changed to return only the components that were specified by the string.
 */

- (void) testParsingStringWithOnlyHourMinuteSecondZulu {
	NSTimeInterval hour = 14.0, minute = 23.0, second = 56.0;
	NSTimeZone *timeZone = [NSTimeZone timeZoneWithName:@"UTC"];
	NSString *string = [self dateStringWithHour:hour minute:minute second:second timeZone:timeZone];
	NSTimeInterval timeInterval = [self dateForTodayWithHour:hour minute:minute second:second timeZone:timeZone].timeIntervalSinceReferenceDate;
	[self attemptToParseString:string
		expectTimeIntervalSinceReferenceDate:timeInterval
		expectTimeZoneWithHoursFromGMT:0.0];
}

- (void) testParsingStringWithOnlyHourMinuteZulu {
	NSTimeInterval hour = 14.0, minute = 23.0;
	NSTimeZone *timeZone = [NSTimeZone timeZoneWithName:@"UTC"];
	NSString *string = [self dateStringWithHour:hour minute:minute second:0.0 timeZone:timeZone];
	NSTimeInterval timeInterval = [self dateForTodayWithHour:hour minute:minute second:0.0 timeZone:timeZone].timeIntervalSinceReferenceDate;
	[self attemptToParseString:string
		expectTimeIntervalSinceReferenceDate:timeInterval
		expectTimeZoneWithHoursFromGMT:0.0];
}

- (void) testParsingStringWithOnlyHourMinuteSecondAndTimeZone {
	NSTimeInterval hour = 14.0, minute = 23.0, second = 56.0;
	NSTimeZone *timeZone = [NSTimeZone timeZoneForSecondsFromGMT:(NSInteger)(-8.0 * gSecondsPerHour)];
	NSString *string = [self dateStringWithHour:hour minute:minute second:second timeZone:timeZone];
	NSTimeInterval timeInterval = [self dateForTodayWithHour:hour minute:minute second:second timeZone:timeZone].timeIntervalSinceReferenceDate;
	[self attemptToParseString:string
		expectTimeIntervalSinceReferenceDate:timeInterval
		expectTimeZoneWithHoursFromGMT:-8.0];
}

- (void) testParsingStringWithOnlyHourMinuteAndTimeZone {
	NSTimeInterval hour = 14.0, minute = 23.0;
	NSTimeZone *timeZone = [NSTimeZone timeZoneForSecondsFromGMT:(NSInteger)(-8.0 * gSecondsPerHour)];
	NSString *string = [self dateStringWithHour:hour minute:minute second:0.0 timeZone:timeZone];
	NSTimeInterval timeInterval = [self dateForTodayWithHour:hour minute:minute second:0.0 timeZone:timeZone].timeIntervalSinceReferenceDate;
	[self attemptToParseString:string
		expectTimeIntervalSinceReferenceDate:timeInterval
		expectTimeZoneWithHoursFromGMT:-8.0];
}

@end
