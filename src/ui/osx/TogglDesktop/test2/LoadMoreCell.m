//
//  LoadMoreCell.m
//  TogglDesktop
//
//  Created by Indrek Vändrik on 19/02/16.
//  Copyright © 2016 Alari. All rights reserved.
//

#import "LoadMoreCell.h"

@implementation LoadMoreCell
extern void *ctx;

- (void)initCell
{
	[self.loadButton setTitle:@"Load More"];
	[self.loader stopAnimation:nil];
	[self.loader setHidden:YES];
}

- (IBAction)loadMoreClicked:(id)sender
{
	[self.loadButton setTitle:@""];
	[self.loader startAnimation:sender];
	[self.loader setHidden:NO];
	toggl_load_more(ctx);
}

@end
