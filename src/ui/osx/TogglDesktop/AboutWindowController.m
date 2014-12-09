//
//  AboutWindowController.m
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 29/10/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "AboutWindowController.h"
#import "UIEvents.h"
#import "toggl_api.h"
#import "DisplayCommand.h"
#import "Utils.h"
#import "Sparkle.h"

@implementation AboutWindowController

extern void *ctx;

- (void)windowDidLoad
{
	[super windowDidLoad];

	NSDictionary *infoDict = [[NSBundle mainBundle] infoDictionary];
	NSString *version = [infoDict objectForKey:@"CFBundleShortVersionString"];
	[self.versionTextField setStringValue:[NSString stringWithFormat:@"Version %@", version]];
	NSString *appname = [infoDict objectForKey:@"CFBundleName"];
	[self.appnameTextField setStringValue:appname];

	NSString *path = [[NSBundle mainBundle] pathForResource:@"Credits" ofType:@"rtf"];
	[self.creditsTextView readRTFDFromFile:path];

	self.windowHasLoad = YES;

	char *str = toggl_get_update_channel(ctx);
	self.updateChannelComboBox.stringValue = [NSString stringWithUTF8String:str];
	free(str);

	[[SUUpdater sharedUpdater] setDelegate:self];

	[[SUUpdater sharedUpdater] resetUpdateCycle];

	[[SUUpdater sharedUpdater] checkForUpdates:self];
}

- (BOOL)isVisible
{
	if (!self.windowHasLoad)
	{
		return NO;
	}
	return self.window.isVisible;
}

- (IBAction)updateChannelSelected:(id)sender
{
	NSString *updateChannel = self.updateChannelComboBox.stringValue;

	toggl_set_update_channel(ctx, [updateChannel UTF8String]);

	[Utils setUpdaterChannel:updateChannel];

	[[SUUpdater sharedUpdater] resetUpdateCycle];

	[[SUUpdater sharedUpdater] checkForUpdates:self];
}

@end
