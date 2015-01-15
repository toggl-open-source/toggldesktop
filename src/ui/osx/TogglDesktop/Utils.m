//
//  Utils.m
//  TogglDesktop
//
//  Created by Tanel Lebedev on 07/05/14.
//  Copyright (c) 2014 Toggl Desktop developers. All rights reserved.
//

#import "Utils.h"

#include "toggl_api.h"

extern void *ctx;

@implementation ScriptResult

- (void)append:(NSString *)moreText
{
	if (!self.text)
	{
		self.text = @"";
	}
	self.text = [self.text stringByAppendingString:moreText];
}

- (NSString *)description
{
	return [NSString stringWithFormat:@"err: %lld, text: %@",
			self.err, self.text];
}

@end

@implementation Utils

+ (ScriptResult *)runScript:(NSString *)script
{
	ScriptResult *result = nil;
	char *text = 0;

	@try {
		int64_t err = 0;
		text = toggl_run_script(ctx, [script UTF8String], &err);
		result = [[ScriptResult alloc] init];
		result.err = err;
		result.text = [NSString stringWithUTF8String:text];
	}
	@catch (NSException *e) {
		NSLog(@"Script exception: %@", e);
	} @finally {
		free(text);
		NSLog(@"Script result: %@", result);
	}

	return result;
}

+ (NSInteger)boolToState:(BOOL)value
{
	if (value)
	{
		return NSOnState;
	}
	return NSOffState;
}

+ (unsigned int)stateToBool:(NSInteger)state
{
	if (NSOnState == state)
	{
		return 1;
	}
	return 0;
}

+ (void)disallowDuplicateInstances
{
	if ([[NSRunningApplication runningApplicationsWithBundleIdentifier:
		  [[NSBundle mainBundle] bundleIdentifier]] count] > 1)
	{
		NSString *msg = [NSString
						 stringWithFormat:@"Another copy of %@ is already running.",
						 [[NSBundle mainBundle]
						  objectForInfoDictionaryKey:(NSString *)kCFBundleNameKey]];
		[[NSAlert alertWithMessageText:msg
						 defaultButton:nil
					   alternateButton:nil
						   otherButton:nil
			 informativeTextWithFormat:@"This copy will now quit."] runModal];

		[NSApp terminate:nil];
	}
}

+ (NSString *)applicationSupportDirectory:(NSString *)environment
{
	NSString *path;
	NSError *error;
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);

	if ([paths count] == 0)
	{
		NSLog(@"Unable to access application support directory!");
	}
	path = [paths[0] stringByAppendingPathComponent:@"Kopsik"];

	// Append environment name to path. So we can have
	// production and development running side by side.
	path = [path stringByAppendingPathComponent:environment];

	if ([[NSFileManager defaultManager] fileExistsAtPath:path])
	{
		return path;
	}
	if (![[NSFileManager defaultManager] createDirectoryAtPath:path
								   withIntermediateDirectories:YES
													attributes:nil
														 error:&error])
	{
		NSLog(@"Create directory error: %@", error);
	}
	return path;
}

/*
 * Returns whether or not an NSString represents a numeric value.
 * For more info see:  http://appliedsoftwaredesign.com/blog/iphone-sdk-nsstring-numeric/
 */
+ (bool)isNumeric:(NSString *)checkText
{
	NSNumberFormatter *numberFormatter = [[NSNumberFormatter alloc] init];
	NSNumber *number = [numberFormatter numberFromString:checkText];

	return number != nil;
}

+ (void)addUnderlineToTextField:(NSTextField *)field
{
	NSMutableAttributedString *forgot = [[field attributedStringValue] mutableCopy];

	[forgot addAttribute:NSUnderlineStyleAttributeName value:[NSNumber numberWithInt:NSUnderlineStyleSingle] range:NSMakeRange(0, forgot.length)];
	[field setAttributedStringValue:forgot];
}

@end

// See https://codereview.chromium.org/7497056/patch/2002/4002 for inspiration
BOOL wasLaunchedAsLoginOrResumeItem()
{
	ProcessSerialNumber psn = { 0, kCurrentProcess };
	NSDictionary *process_info = CFBridgingRelease(ProcessInformationCopyDictionary(&psn, kProcessDictionaryIncludeAllInformationMask));

	long long temp = [[process_info objectForKey:@"ParentPSN"] longLongValue];
	ProcessSerialNumber parent_psn = { (temp >> 32) & 0x00000000FFFFFFFFLL, temp & 0x00000000FFFFFFFFLL };

	NSDictionary *parent_info = CFBridgingRelease(ProcessInformationCopyDictionary(&parent_psn,
																				   kProcessDictionaryIncludeAllInformationMask));

	return [[parent_info objectForKey:@"FileCreator"] isEqualToString:@"lgnw"];
}

// See https://codereview.chromium.org/7497056/patch/2002/4002 for inspiration
BOOL wasLaunchedAsHiddenLoginItem()
{
	if (!wasLaunchedAsLoginOrResumeItem())
	{
		return NO;
	}

	LSSharedFileListRef login_items = LSSharedFileListCreate(NULL,
															 kLSSharedFileListSessionLoginItems,
															 NULL);

	if (!login_items)
	{
		return NO;
	}

	CFArrayRef login_items_array = LSSharedFileListCopySnapshot(login_items,
																NULL);

	CFURLRef url_ref = (__bridge CFURLRef)[NSURL fileURLWithPath:[[NSBundle mainBundle] bundlePath]];

	for (int i = 0; i < CFArrayGetCount(login_items_array); i++)
	{
		LSSharedFileListItemRef item = (LSSharedFileListItemRef)CFArrayGetValueAtIndex(login_items_array, i);
		CFURLRef item_url_ref = NULL;
		if (!LSSharedFileListItemResolve(item, 0, &item_url_ref, NULL) == noErr)
		{
			continue;
		}
		if (CFEqual(item_url_ref, url_ref))
		{
			CFBooleanRef hidden = LSSharedFileListItemCopyProperty(item, kLSSharedFileListLoginItemHidden);
			return (hidden && kCFBooleanTrue == hidden);
		}
	}

	return NO;
}

