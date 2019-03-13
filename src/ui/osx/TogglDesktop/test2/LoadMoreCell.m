//
//  LoadMoreCell.m
//  TogglDesktop
//
//  Created by Indrek Vändrik on 19/02/16.
//  Copyright © 2016 Alari. All rights reserved.
//

#import "LoadMoreCell.h"

@interface LoadMoreCell ()
@property (weak) IBOutlet NSProgressIndicator *loader;
@property (weak) IBOutlet NSButton *loadButton;
- (IBAction)loadMoreClicked:(id)sender;
@end

@implementation LoadMoreCell
extern void *ctx;

- (void)awakeFromNib {
	[super awakeFromNib];

	[self setLoading:NO];
}

- (void)prepareForReuse
{
	[super prepareForReuse];

	[self setLoading:NO];
}

- (IBAction)loadMoreClicked:(id)sender
{
	[self setLoading:YES];
	toggl_load_more(ctx);
}

- (void)setLoading:(BOOL)start
{
	if (start)
	{
		[self.loadButton setTitle:@""];
		[self.loader startAnimation:nil];
		[self.loader setHidden:NO];
	}
	else
	{
		[self.loadButton setTitle:@"Load more"];
		[self.loader stopAnimation:nil];
		[self.loader setHidden:YES];
	}
}

@end
