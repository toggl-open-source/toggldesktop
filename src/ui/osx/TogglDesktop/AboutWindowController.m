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

	[self.window setBackgroundColor:[NSColor colorWithPatternImage:[NSImage imageNamed:@"toggl-desktop-bg.png"]]];

	NSDictionary *infoDict = [[NSBundle mainBundle] infoDictionary];
	NSString *version = [infoDict objectForKey:@"CFBundleShortVersionString"];
	[self.versionTextField setStringValue:[NSString stringWithFormat:@"Version %@", version]];
	NSString *appname = [infoDict objectForKey:@"CFBundleName"];
	[self.appnameTextField setStringValue:appname];

	NSString *path = [[NSBundle mainBundle] pathForResource:@"Credits" ofType:@"rtf"];
	[self.creditsTextView readRTFDFromFile:path];

	self.windowHasLoad = YES;
	self.restart = NO;

	char *str = toggl_get_update_channel(ctx);
	self.updateChannelComboBox.stringValue = [NSString stringWithUTF8String:str];
	free(str);

	if ([self updateCheckEnabled])
	{
		self.updateChannelComboBox.hidden = NO;
		self.updateChannelLabel.hidden = NO;

		if (![[SUUpdater sharedUpdater] updateInProgress])
		{
			[self checkForUpdates];
		}

		[self displayUpdateStatus];
	}
}

- (BOOL)updateCheckEnabled
{
	NSDictionary *infoDict = [[NSBundle mainBundle] infoDictionary];

	return [infoDict[@"KopsikCheckForUpdates"] boolValue];
}

- (void)checkForUpdates
{
	if (![self updateCheckEnabled])
	{
		return;
	}
	[[SUUpdater sharedUpdater] resetUpdateCycle];
	[[SUUpdater sharedUpdater] checkForUpdatesInBackground];
}

- (void)updater:(SUUpdater *)updater willInstallUpdateOnQuit:(SUAppcastItem *)item immediateInstallationInvocation:(NSInvocation *)invocation
{
	NSLog(@"Download finished: %@", item.displayVersionString);

	self.updateStatus =
		[NSString stringWithFormat:@"Restart app to upgrade to %@",
		 item.displayVersionString];

	[self displayUpdateStatus];
	[self.restartButton setHidden:NO];
}

- (void)updater:(SUUpdater *)updater didFindValidUpdate:(SUAppcastItem *)update
{
	NSLog(@"update found: %@", update.displayVersionString);

	self.updateStatus =
		[NSString stringWithFormat:@"Update found: %@. Downloading..",
		 update.displayVersionString];

	[self.restartButton setHidden:YES];
	[self displayUpdateStatus];
	[Utils runClearCommand];
}

- (void)displayUpdateStatus
{
	NSLog(@"automaticallyDownloadsUpdates=%d", [[SUUpdater sharedUpdater] automaticallyDownloadsUpdates]);

	if (self.updateStatus)
	{
		self.updateStatusTextField.stringValue = self.updateStatus;
		[self.updateStatusTextField setHidden:NO];
	}
	else
	{
		[self.updateStatusTextField setHidden:YES];
	}
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

	[self checkForUpdates];
}

- (IBAction)clickRestartButton:(id)sender;
{
	self.restart = YES;
	[[NSApplication sharedApplication] terminate:nil];
}

- (void)updaterDidNotFindUpdate:(SUUpdater *)update
{
	NSLog(@"No update found");
}

- (void)updater:(SUUpdater *)updater willInstallUpdate:(SUAppcastItem *)update
{
	NSLog(@"Will install update %@", update.displayVersionString);
}

- (void)updater:(SUUpdater *)updater didAbortWithError:(NSError *)error
{
	NSLog(@"Update check failed with error %@", error);
}

@end
