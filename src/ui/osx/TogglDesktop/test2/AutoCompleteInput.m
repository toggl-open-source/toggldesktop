//
//  AutoComleteInput.m
//  LiteComplete
//
//  Created by Indrek Vändrik on 20/02/2018.
//  Copyright © 2018 Toggl. All rights reserved.
//

#import "AutoCompleteInput.h"

@implementation AutoCompleteInput

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];
    
    // Drawing code here.
}

- (id)initWithCoder:(NSCoder *)coder
{
    self = [super initWithCoder:coder];
    if (self)
    {
        //self.rect = NSMakeRect(0, 0, 0, 0);
        self.posY = 0;
        NSLog(@"// ** Create ViewItems");
        viewitems = [NSMutableArray array];
        [viewitems addObject:@"Test1"];
        [viewitems addObject:@"Test2"];
        [viewitems addObject:@"Test3"];
        [viewitems addObject:@"Test4"];
        [viewitems addObject:@"Test5"];
        
        NSRect s = self.frame;
        int h = 200;
        self.autocompleteTableContainer = [[NSScrollView alloc] initWithFrame:NSMakeRect(s.origin.x, s.origin.y-h, s.size.width, h)];
        self.nibAutoCompleteTableCell = [[NSNib alloc] initWithNibNamed:@"AutoCompleteTableCell" bundle:nil];
        self.autocompleteTableView = [[AutoCompleteTable alloc] initWithFrame:NSMakeRect(0,0,0,0)];
        [self.autocompleteTableView registerNib:self.nibAutoCompleteTableCell
                                forIdentifier :@"AutoCompleteTableCell"];

        [self.autocompleteTableView setDelegate:self];
        [self.autocompleteTableView setDataSource:self];

        [self.autocompleteTableContainer setDocumentView:self.autocompleteTableView];
        [self.autocompleteTableContainer setAutohidesScrollers:YES];
        [self.autocompleteTableContainer setHasVerticalScroller:YES];
    }
    return self;
}

- (void)setPos:(int)posy
{
    self.posY = posy;
}

- (void)toggleTableView:(BOOL)show
{
    NSLog(@"// ** Add table");
    if (show == YES) {
        if (self.autocompleteTableView.listVisible == NO) {
            [self.window.contentView addSubview:self.autocompleteTableContainer positioned:NSWindowAbove relativeTo:nil];
            self.autocompleteTableView.listVisible = YES;
        }
        [self.autocompleteTableView reloadData];
    } else {
        self.autocompleteTableView.listVisible = NO;
        [self.autocompleteTableContainer removeFromSuperview];
    }
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tv
{
    int result = 0;
    result = (int)[[self stringValue] length];
    if (result < (int)[viewitems count]) {
        [self updateDropdownHeight:result];
        return result;
    }
    @synchronized(viewitems)
    {
        result = (int)[viewitems count];
    }
    NSLog(@"----- ROWS: %d", result);
    
    return result;
}

-(void)updateDropdownHeight:(int)count
{
    int itemHeight = 35;
    int h = count * itemHeight;
    [self.autocompleteTableContainer setFrame: NSMakeRect(
                                                          self.frame.origin.x,
                                                          self.frame.origin.y-h+self.posY,
                                                          self.frame.size.width,
                                                          h)];
}

/*
- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {
    return [NSString stringWithFormat:@"%ld", row];
}

- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {
    NSTextField *textField = [[NSTextField alloc] initWithFrame:(NSRect){.size = {100, 15}}];
    textField.stringValue = [NSString stringWithFormat:@"%ld", row];
    return textField;
}
*/


- (BOOL)  tableView:(NSTableView *)aTableView
    shouldSelectRow:(NSInteger)rowIndex
{
    return YES;
}
 - (NSView *) tableView:(NSTableView *)tableView
 viewForTableColumn:(NSTableColumn *)tableColumn
 row:(NSInteger)row
 {
     NSString *item = @"";
 
     @synchronized(viewitems)
     {
         item = [viewitems objectAtIndex:row];
     }
 
     AutoCompleteTableCell *cell = [tableView makeViewWithIdentifier:@"AutoCompleteTableCell"
                                                               owner:self];
     [cell render:item];
     return cell;
     
 }


- (CGFloat)tableView:(NSTableView *)tableView
         heightOfRow:(NSInteger)row
{
    NSString *item = @"";
    
    @synchronized(viewitems)
    {
        if (row < viewitems.count)
        {
            item = viewitems[row];
        }
    }
    return 30;
}

-(void)keyUp:(NSEvent *)event {
    NSLog(@"EventCode: %hu", [event keyCode]);
    if ([event keyCode] == kVK_DownArrow) {
        if (self.autocompleteTableView.listVisible == NO) {
            [self toggleTableView:YES];
        }
        [[self window] makeFirstResponder:self.autocompleteTableView];
        [self.autocompleteTableView setFirstRowAsSelected];
    }
}

@end
