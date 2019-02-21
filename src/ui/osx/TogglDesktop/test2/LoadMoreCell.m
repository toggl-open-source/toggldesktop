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
