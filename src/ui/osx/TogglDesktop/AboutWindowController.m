//
//  AboutWindowController.m
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 29/10/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "AboutWindowController.h"
#import "Update.h"
#import "UIEvents.h"
#import "toggl_api.h"
#import "DisplayCommand.h"
#import "Utils.h"

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

	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(startDisplayUpdate:)
												 name:kDisplayUpdate
											   object:nil];

	[self displayUpdate:self.displayCommand];

	self.windowHasLoad = YES;
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
}

- (IBAction)showWindow:(id)sender
{
	[super showWindow:sender];
}

- (IBAction)checkForUpdateClicked:(id)sender
{
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:self.displayCommand.update.URL]];
	[[NSApplication sharedApplication] terminate:self];
}

- (void)startDisplayUpdate:(NSNotification *)notification
{
	[self performSelectorOnMainThread:@selector(displayUpdate:)
						   withObject:notification.object
						waitUntilDone:NO];
}

- (void)displayUpdate:(DisplayCommand *)cmd
{
	NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

	self.displayCommand = cmd;

	self.updateChannelComboBox.stringValue = self.displayCommand.update.channel;

	if (self.displayCommand.open)
	{
		[self showWindow:self];
	}

	if (self.displayCommand.update.is_checking)
	{
		[self.checkForUpdateButton setEnabled:NO];
		[self.updateChannelComboBox setEnabled:NO];
		[self.checkForUpdateButton setTitle:@"Checking for update.."];
		return;
	}

	[self.updateChannelComboBox setEnabled:YES];

	if (self.displayCommand.update.is_update_available)
	{
		self.checkForUpdateButton.title =
			[NSString stringWithFormat:@"Click here to download update! (%@)",
			 self.displayCommand.update.version];
		[self.checkForUpdateButton setEnabled:YES];
	}
	else
	{
		self.checkForUpdateButton.title = @"TogglDesktop is up to date.";
		[self.checkForUpdateButton setEnabled:NO];
	}
}

@end
