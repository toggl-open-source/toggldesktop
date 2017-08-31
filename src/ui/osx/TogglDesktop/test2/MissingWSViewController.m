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
	// Do view setup here.
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
