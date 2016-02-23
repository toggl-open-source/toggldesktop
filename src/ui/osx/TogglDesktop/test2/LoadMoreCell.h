//
//  LoadMoreCell.h
//  TogglDesktop
//
//  Created by Indrek Vändrik on 19/02/16.
//  Copyright © 2016 Alari. All rights reserved.
//

#import "TimeEntryCell.h"

@interface LoadMoreCell : TimeEntryCell
@property BOOL isHeader;
@property (weak) IBOutlet NSProgressIndicator *loader;
- (void)initCell;
- (IBAction)loadMoreClicked:(id)sender;
@property (weak) IBOutlet NSButton *loadButton;
@end
