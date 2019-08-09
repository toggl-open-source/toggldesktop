//
//  OverlayViewController.m
//  TogglDesktop
//
//  Created by Indrek Vändrik on 31/08/2017.
//  Copyright © 2017 Toggl Desktop Developers. All rights reserved.
//

#import "OverlayViewController.h"
#import "NSTextFieldClickablePointer.h"

@interface OverlayViewController () <NSTextFieldDelegate>
@property (weak) IBOutlet NSView *tosContainerView;
@property (weak) IBOutlet NSButton *actionButton;
@property (weak) IBOutlet NSTextFieldClickablePointer *bottomLink;
@property (strong) IBOutlet NSTextView *mainText;
@property (weak) IBOutlet NSView *workspaceContainerView;
@property (weak) IBOutlet NSTextFieldClickablePointer *createNewWorkspaceBtn;
@property (weak) IBOutlet NSTextFieldClickablePointer *continueTrackingLabel;
@property (weak) IBOutlet NSTextFieldClickablePointer *createWorkspaceBtn;
@property (weak) IBOutlet NSTextFieldClickablePointer *forceSyncBtn;
@property (assign, nonatomic) OverlayDisplayType displayType;
@end

@implementation OverlayViewController

extern void *ctx;

- (void)awakeFromNib
{
	[super awakeFromNib];

	[self initCommon];
	[self setupTos];
	[self setupMissingWS];
}

- (void)initCommon
{
	self.displayType = OverlayDisplayTypeWorkspace;
	self.workspaceContainerView.hidden = YES;
	self.tosContainerView.hidden = YES;
	self.forceSyncBtn.delegate = self;
	self.continueTrackingLabel.delegate = self;
	self.createWorkspaceBtn.delegate = self;
}

- (IBAction)actionClicked:(id)sender
{
	toggl_accept_tos(ctx);
}

- (void)setType:(int)type
{
	self.displayType = type;
	self.workspaceContainerView.hidden = YES;
	self.tosContainerView.hidden = YES;
	switch (self.displayType)
	{
		case OverlayDisplayTypeWorkspace :
			self.workspaceContainerView.hidden = NO;
			break;
		case OverlayDisplayTypeTOS :
			self.tosContainerView.hidden = NO;
			break;
	}
}

- (void)setupMissingWS
{
	self.forceSyncBtn.titleUnderline = YES;
	self.createWorkspaceBtn.titleUnderline = YES;
}

- (void)setupTos
{
	[self.actionButton setTitle:@"I accept"];

	// title text
	NSMutableParagraphStyle *topParagrapStyle = NSMutableParagraphStyle.new;
	topParagrapStyle.alignment = kCTTextAlignmentCenter;

	NSMutableParagraphStyle *paragrapStyle = NSMutableParagraphStyle.new;
	paragrapStyle.alignment = kCTTextAlignmentCenter;
	paragrapStyle.lineHeightMultiple = 1.5;

	NSMutableAttributedString *result = [[NSMutableAttributedString alloc] initWithString:@""];

	NSMutableAttributedString *string = [[NSMutableAttributedString alloc] initWithString:@"Our Terms of Service and Privacy Policy have changed\n\n"];

	[string setAttributes:
	 @{
		 NSForegroundColorAttributeName:[NSColor labelColor],
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
		 NSForegroundColorAttributeName:[NSColor labelColor],
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
		 NSForegroundColorAttributeName:[NSColor labelColor],
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
		 NSForegroundColorAttributeName:[NSColor labelColor],
		 NSParagraphStyleAttributeName: paragrapStyle
	 }
					range:NSMakeRange(0, [string length])];    [result appendAttributedString:string];

	[self.mainText setTextColor:[NSColor labelColor]];
	self.mainText.linkTextAttributes = @{ NSForegroundColorAttributeName:[NSColor labelColor] };

	[[self.mainText textStorage] setAttributedString:result];


	// bottom text
	NSMutableAttributedString *bottomString = [[NSMutableAttributedString alloc] initWithString:@"If you have any questions, contact us at "];

	NSMutableAttributedString *mail = [[NSMutableAttributedString alloc] initWithString:@"support@toggl.com"];

	[mail setAttributes:
	 @{
		 NSForegroundColorAttributeName:[NSColor labelColor],
		 NSFontAttributeName : [NSFont boldSystemFontOfSize:12]
	 }
				  range:NSMakeRange(0, [mail length])];

	[bottomString appendAttributedString:mail];
	[bottomString addAttribute:NSParagraphStyleAttributeName value:paragrapStyle range:NSMakeRange(0, string.length)];

	[self.bottomLink setAttributedStringValue:bottomString];
}

- (IBAction)loginBtnOnTap:(id)sender
{
	toggl_open_in_browser(ctx);
}

- (void)textFieldClicked:(id)sender
{
	if (sender == self.forceSyncBtn)
	{
		toggl_fullsync(ctx);
		return;
	}

	if (sender == self.createWorkspaceBtn || sender == self.continueTrackingLabel)
	{
		toggl_open_in_browser(ctx);
		return;
	}
}

@end
