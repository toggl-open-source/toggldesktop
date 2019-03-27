//
//  TimelineChunkCircle.h
//  TogglDesktop
//
//  Created by Indrek Vändrik on 27/03/2019.
//  Copyright © 2019 Toggl Desktop Developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

@interface TimelineChunkCircle : NSView
@property int value;
- (void)setNewValue:(int)newValue;
@end

NS_ASSUME_NONNULL_END
