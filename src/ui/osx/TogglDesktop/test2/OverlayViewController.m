//
//  OverlayViewController.m
//  TogglDesktop
//
//  Created by Indrek Vändrik on 31/08/2017.
//  Copyright © 2017 Toggl Desktop Developers. All rights reserved.
//

#import "OverlayViewController.h"

@interface OverlayViewController ()
@property int currentType;
@end

@implementation OverlayViewController

extern void *ctx;

- (void)loadView
{
	[super loadView];
	NSOperatingSystemVersion version = [[NSProcessInfo processInfo] operatingSystemVersion];
	if (version.minorVersion < 11)
	{
		[self viewDidLoad];
	}
}

- (void)viewDidLoad
{
	[super viewDidLoad];
}

- (IBAction)actionClicked:(id)sender
{
	if (self.currentType == 0)
	{
		toggl_open_in_browser(ctx);
	}
	else
	{
		toggl_accept_tos(ctx);
	}
}

- (void)textFieldClicked:(id)sender
{
	if (self.currentType == 0 && sender == self.bottomLink)
	{
		toggl_fullsync(ctx);
	}
}

- (void)setType:(int)type
{
	self.currentType = type;

	if (self.currentType == 0)
	{
		// missing ws
		[self setupMissingWS];
	}
	else if (self.currentType == 1)
	{
		// tos accept
		[self setupTos];
	}
}

- (void)setupMissingWS
{
	NSMutableAttributedString *result =
		[[NSMutableAttributedString alloc] initWithString:@"You no longer have access to your last Workspace. Create a new workspace on Toggl.com to continue tracking"];

	[result setAttributes:
	 @{
		 NSFontAttributeName : [NSFont systemFontOfSize:15],
		 NSForegroundColorAttributeName:[NSColor highlightColor]
	 }
					range:NSMakeRange(0, [result length])];

	[self.actionButton setTitle:@"Log in to Toggl.com"];
	[[self.mainText textStorage] setAttributedString:result];

	// Setup up text underline for "Force sync" link

	NSMutableParagraphStyle *paragrapStyle = NSMutableParagraphStyle.new;
	paragrapStyle.alignment                = kCTTextAlignmentCenter;

	NSMutableAttributedString *string =
		[[NSMutableAttributedString alloc] initWithString:@"Created your new workspace?\n"];

	NSMutableAttributedString *sync =
		[[NSMutableAttributedString alloc] initWithString:@"Force sync"];

	[sync addAttribute:NSUnderlineStyleAttributeName value:[NSNumber numberWithInt:NSUnderlineStyleSingle] range:NSMakeRange(0, sync.length)];

	[string appendAttributedString:sync];
	[string addAttribute:NSParagraphStyleAttributeName value:paragrapStyle range:NSMakeRange(0, string.length)];

	[self.bottomLink setAttributedStringValue:string];
}

- (void)setupTos
{
	[self.actionButton setTitle:@"I accept"];

	// title text
	NSMutableParagraphStyle *topParagrapStyle = NSMutableParagraphStyle.new;
	topParagrapStyle.alignment                = kCTTextAlignmentCenter;

	NSMutableParagraphStyle *paragrapStyle = NSMutableParagraphStyle.new;
	paragrapStyle.alignment                = kCTTextAlignmentCenter;
	paragrapStyle.lineHeightMultiple = 1.5;

	NSMutableAttributedString *result = [[NSMutableAttributedString alloc] initWithString:@""];

	NSMutableAttributedString *string = [[NSMutableAttributedString alloc] initWithString:@"Our Terms of Service and Privacy Policy have changed\n\n"];

	[string setAttributes:
	 @{
		 NSForegroundColorAttributeName:[NSColor whiteColor],
		 NSFontAttributeName: [NSFont systemFontOfSize:20],
		 NSParagraphStyleAttributeName: topParagrapStyle
	 }
					range:NSMakeRange(0, [string length])];

	[result appendAttributedString:string];

	// content

	NSURL *tosUrl = [NSURL URLWithString:@"https://toggl.com/legal/terms"];
	NSURL *privacyUrl = [NSURL URLWithString:@"https://toggl.com/legal/privacy"];

	string = [[NSMutableAttributedString alloc] initWithString:@"Please read and accept our updated "];
	[string setAttributes:
	 @{
		 NSForegroundColorAttributeName:[NSColor whiteColor],
		 NSParagraphStyleAttributeName: paragrapStyle
	 }
					range:NSMakeRange(0, [string length])];
	[result appendAttributedString:string];

	// terms of service url
	string = [[NSMutableAttributedString alloc] initWithString:@"Terms of Service"];
	[string setAttributes:
	 @{
		 NSLinkAttributeName: tosUrl,
		 NSUnderlineStyleAttributeName: [NSNumber numberWithInt:NSUnderlineStyleSingle]
	 }
					range:NSMakeRange(0, [string length])];

	[result appendAttributedString:string];

	string = [[NSMutableAttributedString alloc] initWithString:@" and "];
	[string setAttributes:
	 @{
		 NSForegroundColorAttributeName:[NSColor whiteColor],
		 NSParagraphStyleAttributeName: paragrapStyle
	 }
					range:NSMakeRange(0, [string length])];    [result appendAttributedString:string];

	// privacy url
	string = [[NSMutableAttributedString alloc] initWithString:@"Privacy Policy"];

	[string setAttributes:
	 @{
		 NSLinkAttributeName: privacyUrl,
		 NSUnderlineStyleAttributeName: [NSNumber numberWithInt:NSUnderlineStyleSingle]
	 }
					range:NSMakeRange(0, [string length])];
	[result appendAttributedString:string];

	string = [[NSMutableAttributedString alloc] initWithString:@" to continue using Toggl."];
	[string setAttributes:
	 @{
		 NSForegroundColorAttributeName:[NSColor whiteColor],
		 NSParagraphStyleAttributeName: paragrapStyle
	 }
					range:NSMakeRange(0, [string length])];    [result appendAttributedString:string];

	[self.mainText setTextColor:[NSColor whiteColor]];
	self.mainText.linkTextAttributes = @{ NSForegroundColorAttributeName:[NSColor whiteColor] };

	[[self.mainText textStorage] setAttributedString:result];


	// bottom text
	NSMutableAttributedString *bottomString = [[NSMutableAttributedString alloc] initWithString:@"If you have any questions, contact us at "];

	NSMutableAttributedString *mail = [[NSMutableAttributedString alloc] initWithString:@"support@toggl.com"];

	[mail setAttributes:
	 @{
		 NSForegroundColorAttributeName:[NSColor whiteColor],
		 NSFontAttributeName : [NSFont boldSystemFontOfSize:12]
	 }
				  range:NSMakeRange(0, [mail length])];

	[bottomString appendAttributedString:mail];
	[bottomString addAttribute:NSParagraphStyleAttributeName value:paragrapStyle range:NSMakeRange(0, string.length)];

	[self.bottomLink setAttributedStringValue:bottomString];
}

@end
