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
#import "UnsupportedNotice.h"
#import "NSTextFieldClickablePointer.h"
#import "ConvertHexColor.h"
#import "TogglDesktop-Swift.h"

static NSString *const kTogglDesktopGithubURL = @"https://github.com/toggl/toggldesktop";

typedef enum : NSUInteger
{
	DownloadStateNone,
	DownloadStateDownloading,
	DownloadStateRestart
} DownloadState;

@interface AboutWindowController () <NSTextFieldDelegate>
@property (weak) IBOutlet NSTextField *appnameTextField;
@property (weak) IBOutlet NSTextField *versionTextField;
@property (weak) IBOutlet NSTextField *updateStatusTextField;
@property (weak) IBOutlet NSComboBox *updateChannelComboBox;
@property (weak) IBOutlet NSTextField *updateChannelLabel;
@property (weak) IBOutlet FlatButton *restartButton;
@property (weak) IBOutlet NSTextFieldClickablePointer *findUsInGithub;
@property (weak) IBOutlet NSBox *boxView;
@property (assign, nonatomic) DownloadState downloadState;
@end

@implementation AboutWindowController

extern void *ctx;

- (void)windowDidLoad
{
	[super windowDidLoad];

	// Clean window titlebar
	self.window.titleVisibility = NSWindowTitleHidden;
	self.window.titlebarAppearsTransparent = YES;
	self.window.styleMask |= NSFullSizeContentViewWindowMask;

	[self.window setBackgroundColor:[NSColor colorWithPatternImage:[NSImage imageNamed:@"background-pattern"]]];

	NSDictionary *infoDict = [[NSBundle mainBundle] infoDictionary];
	NSString *version = [infoDict objectForKey:@"CFBundleShortVersionString"];
	[self.versionTextField setStringValue:[NSString stringWithFormat:@"Version %@", version]];

	self.windowHasLoad = YES;
	self.restart = NO;

	[self initCommon];
}

- (void)initCommon {
	self.findUsInGithub.delegate = self;

	self.findUsInGithub.titleUnderline = YES;

	self.boxView.wantsLayer = YES;
	self.boxView.layer.masksToBounds = NO;
	self.boxView.shadow = [[NSShadow alloc] init];
	self.boxView.layer.shadowColor = [NSColor colorWithWhite:0 alpha:0.1].CGColor;
	self.boxView.layer.shadowOpacity = 1.0;
	self.boxView.layer.shadowOffset = CGSizeMake(0, -2);
	self.boxView.layer.shadowRadius = 6;

	[self setDownloadState:self.downloadState];
}

- (BOOL)isVisible
{
	if (!self.windowHasLoad)
	{
		return NO;
	}
	return self.window.isVisible;
}

- (IBAction)clickRestartButton:(id)sender;
{
	self.restart = YES;
	[[NSApplication sharedApplication] terminate:nil];
}

- (void)textFieldClicked:(id)sender
{
	if (sender == self.findUsInGithub)
	{
		[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:kTogglDesktopGithubURL]];
		return;
	}
}

- (void)setDownloadState:(DownloadState)downloadState
{
	_downloadState = downloadState;
}

- (void)renderRestartButton {
	// Render button
	switch (self.downloadState)
	{
		case DownloadStateNone :
			self.restartButton.hidden = YES;
			break;
		case DownloadStateDownloading :
			self.restartButton.enabled = NO;
			self.restartButton.hidden = NO;
			self.restartButton.title = @"Downloading...";
			if (@available(macOS 10.13, *))
			{
				self.restartButton.textColor = [NSColor colorNamed:@"grey-text-color"];
				self.restartButton.bgColor = [NSColor colorWithRed:177.0 / 255.0 green:177.0 / 255.0 blue:177.0 / 255.0 alpha:0.1];
			}
			else
			{
				self.restartButton.textColor = [ConvertHexColor hexCodeToNSColor:@"#d9d9d9"];
				self.restartButton.bgColor = [NSColor colorWithRed:177.0 / 255.0 green:177.0 / 255.0 blue:177.0 / 255.0 alpha:0.1];
			}
			break;
		case DownloadStateRestart :
			self.restartButton.enabled = YES;
			self.restartButton.hidden = NO;
			self.restartButton.title = @"Restart";
			if (@available(macOS 10.13, *))
			{
				self.restartButton.bgColor = [NSColor colorNamed:@"green-color"];
			}
			else
			{
				self.restartButton.bgColor = [ConvertHexColor hexCodeToNSColor:@"#28cd41"];
			}
			self.restartButton.textColor = [NSColor whiteColor];
			break;
	}
}

@end
