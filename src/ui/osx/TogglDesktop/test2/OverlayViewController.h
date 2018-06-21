//
//  OverlayViewController.h
//  TogglDesktop
//
//  Created by Indrek Vändrik on 31/08/2017.
//  Copyright © 2017 Toggl Desktop Developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "NSTextFieldClickablePointer.h"

@interface OverlayViewController : NSViewController
@property (weak) IBOutlet NSButton *actionButton;
@property (weak) IBOutlet NSTextFieldClickablePointer *bottomLink;
@property (strong) IBOutlet NSTextView *mainText;
- (void)setType:(int)type;
@end
