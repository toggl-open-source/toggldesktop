//
//  OverlayViewController.h
//  TogglTrack
//
//  Created by Indrek Vändrik on 31/08/2017.
//  Copyright © 2017 Toggl Track Developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>

typedef NS_ENUM (NSUInteger, OverlayDisplayType)
{
	OverlayDisplayTypeWorkspace,
	OverlayDisplayTypeTOS,
};

@interface OverlayViewController : NSViewController
- (void)setType:(int)type;
@end
