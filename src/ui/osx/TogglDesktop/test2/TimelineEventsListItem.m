//
//  TimelineEventsListItem.m
//  TogglDesktop
//
//  Created by Indrek Vändrik on 05/02/16.
//  Copyright © 2016 Alari. All rights reserved.
//

#import "TimelineEventsListItem.h"

@implementation TimelineEventsListItem

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];
    
    // Drawing code here.
}

- (void)render:(TimelineChunkView *)view_item
{
    NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");
    NSLog(@"TL ITEM: %@", view_item.description);
}

- (void)tester
{
    NSLog(@"TESTERRRR");
}
@end
