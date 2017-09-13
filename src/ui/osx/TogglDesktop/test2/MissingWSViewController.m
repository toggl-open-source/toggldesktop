//
//  MissingWSViewController.m
//  TogglDesktop
//
//  Created by Indrek Vändrik on 31/08/2017.
//  Copyright © 2017 Alari. All rights reserved.
//

#import "MissingWSViewController.h"

@interface MissingWSViewController ()

@end

@implementation MissingWSViewController

extern void *ctx;

- (void)viewDidLoad
{
	[super viewDidLoad];
	// Setup up text underline for "Force sync" link

	NSMutableParagraphStyle *paragrapStyle = NSMutableParagraphStyle.new;

	paragrapStyle.alignment                = kCTTextAlignmentCenter;


	NSMutableAttributedString *string =
		[[NSMutableAttributedString alloc] initWithString:self.syncLink.stringValue];


	NSMutableAttributedString *sync =
		[[NSMutableAttributedString alloc] initWithString:@"Force sync"];

	[sync addAttribute:NSUnderlineStyleAttributeName value:[NSNumber numberWithInt:NSUnderlineStyleSingle] range:NSMakeRange(0, sync.length)];

	[string appendAttributedString:sync];
	[string addAttribute:NSParagraphStyleAttributeName value:paragrapStyle range:NSMakeRange(0, string.length)];

	[self.syncLink setAttributedStringValue:string];
}

- (IBAction)loginClicked:(id)sender
{
	toggl_open_in_browser(ctx);
}

- (void)textFieldClicked:(id)sender
{
	if (sender == self.syncLink)
	{
		toggl_fullsync(ctx);
	}
}

@end
