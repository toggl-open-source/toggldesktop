#import <Foundation/Foundation.h>
#import "ISO8601DateFormatter.h"

static void testFormatStrings(int hour, int minute);

int main(void) {
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	ISO8601DateFormatter *formatter = [[[ISO8601DateFormatter alloc] init] autorelease];
	formatter.includeTime = YES;
	NSDate *date = [NSDate dateWithTimeIntervalSinceReferenceDate:336614400.0];
	NSLog(@"2011-09-01 at 5 PM ET: %@", [formatter stringFromDate:date]);

	testFormatStrings(11, 6);
	testFormatStrings(2, 6);
	testFormatStrings(-2, 6);

	[pool drain];
	return EXIT_SUCCESS;
}

static void testFormatStrings(int hour, int minute) {
	NSArray *formatStrings = [NSArray arrayWithObjects:
		@"%@: %02d:%02d",
		@"%@: %+02d:%02d",
		@"%@: %0+2d:%02d",
		@"%@: %02+d:%02d",
		@"%@: %+.2d:%02d",
		nil];
	NSLog(@"Testing with NSLog:");
	for (NSString *format in formatStrings) {
		NSLog(format, format, hour, minute);
	}
	printf("Testing with printf:\n");
	for (NSString *format in formatStrings) {
		format = [format stringByReplacingOccurrencesOfString:@"%@" withString:@"%s"];
		printf([[format stringByAppendingString:@"\n"] UTF8String], [format UTF8String], hour, minute);
	}
}
