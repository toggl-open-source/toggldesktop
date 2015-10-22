//
//  TimelineViewController.h
//  TogglDesktop
//
//  Created by Tanel Lebedev on 22/10/15.
//  Copyright © 2015 Toggl Desktop Developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface TimelineViewController : NSViewController
@property IBOutlet NSTextField *dateLabel;
@property IBOutlet NSTextField *timeLabel;
@property IBOutlet NSTextField *descriptionText;
@property IBOutlet NSButton *prevButton;
@property IBOutlet NSButton *nextButton;
@property IBOutlet NSButton *createButton;
@property IBOutlet NSScrollView *scrollView;
@end
