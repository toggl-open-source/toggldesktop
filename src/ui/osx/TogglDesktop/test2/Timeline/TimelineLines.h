//
//  TimelineLines.h
//  TogglDesktop
//
//  Created by Indrek Vändrik on 27/03/2019.
//  Copyright © 2019 Toggl Desktop Developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "ConvertHexColor.h"
#import "TimelineChunkView.h"

NS_ASSUME_NONNULL_BEGIN

@interface TimelineLines : NSView

@property NSColor *grayColor;
@property NSColor *blueColor;
@property uint64_t bottom;
@property uint64_t height;
- (void)setNewValue:(TimelineChunkView *)view;
- (void)setNewBottom:(uint64_t)bottom newHeight:(uint64_t)height;

@end

NS_ASSUME_NONNULL_END
