#import <Foundation/Foundation.h>

#import "ISO8601DateFormatter.h"

int main(void) {
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	sleep(1);

	ISO8601DateFormatter *formatter = [[[ISO8601DateFormatter alloc] init] autorelease];
	NSString *inString = @"2011-04-12T13:15:17-0800";
	NSUInteger numResults = 0;
	NSDate *start, *end;
	enum { numReps = 10000 };

	NSLog(@"Timing ISO8601DateFormatter");

	start = [NSDate date];
	for (NSUInteger i = 10000; i > 0; --i) {
		NSDate *date = [formatter dateFromString:inString];
		NSString *outString = [formatter stringFromDate:date];
		if (outString) ++numResults;
	}
	end = [NSDate date];
	NSLog(@"Time taken: %f seconds", [end timeIntervalSinceDate:start]);
	NSLog(@"Number of dates and strings computed: %lu each", (unsigned long)numResults);
	NSLog(@"Time taken per date: %f seconds", [end timeIntervalSinceDate:start] / numReps);

	[pool drain];
	pool = [[NSAutoreleasePool alloc] init];

	sleep(1);

	numResults = 0;

	NSLog(@"Timing C standard library parsing and unparsing");

	struct tm timeInfo;
	time_t then;
	char buffer[80] = { 0 };
	NSTimeInterval timeZoneOffset = [[NSTimeZone localTimeZone] secondsFromGMT];

	start = [NSDate date];
	for (NSUInteger i = 10000; i > 0; --i) {
    	strptime([inString cStringUsingEncoding:NSUTF8StringEncoding], "%Y-%m-%dT%H:%M:%S%z", &timeInfo);
    	timeInfo.tm_isdst = -1;
    	then = mktime(&timeInfo);

		NSDate *date = [NSDate dateWithTimeIntervalSince1970:then + timeZoneOffset];

		struct tm *outputTimeInfo;
		time_t outputTime = [date timeIntervalSince1970] - timeZoneOffset;
		outputTimeInfo = localtime(&outputTime);
		strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S%z", outputTimeInfo);

		NSString *outString = [NSString stringWithCString:buffer encoding:NSUTF8StringEncoding];
		if (outString) ++numResults;
	}
	end = [NSDate date];
	NSLog(@"Time taken: %f seconds", [end timeIntervalSinceDate:start]);
	NSLog(@"Number of dates and strings computed: %lu each", (unsigned long)numResults);
	NSLog(@"Time taken per date: %f seconds", [end timeIntervalSinceDate:start] / numReps);

	sleep(1);

	[pool drain];
	return EXIT_SUCCESS;
}
