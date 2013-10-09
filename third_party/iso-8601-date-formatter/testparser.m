#import <Foundation/Foundation.h>
#import "ISO8601DateFormatter.h"

int main(int argc, const char **argv) {
	NSAutoreleasePool *pool = [NSAutoreleasePool new];

	BOOL parseStrictly = NO;
	if((argc > 1) && (strcmp(argv[1], "--strict") == 0)) {
		--argc;++argv;
		parseStrictly = YES;
	}

	[NSTimeZone setDefaultTimeZone:[NSTimeZone timeZoneForSecondsFromGMT:+0]];

	ISO8601DateFormatter *formatter = [[[ISO8601DateFormatter alloc] init] autorelease];
	formatter.parsesStrictly = parseStrictly;

	while(--argc) {
		NSString *str = [NSString stringWithUTF8String:*++argv];
		NSLog(@"Parsing strictly: %hhi", parseStrictly);
		NSDate *date = [formatter dateFromString:str];
		fputs([[NSString stringWithFormat:@"%@ %C %@\n", str, (unsigned short)0x2192, date] UTF8String], stdout);
	}

	[pool release];
	return 0;
}
