//
//  IdleNotificationWindowController.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 06/11/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import "IdleNotificationWindowController.h"
#import "UIEvents.h"
#import "IdleEvent.h"

@interface IdleNotificationWindowController ()
@end

@implementation IdleNotificationWindowController

- (id)initWithWindow:(NSWindow *)window
{
    self = [super initWithWindow:window];
    if (self) {
        // Initialization code here.
    }
    return self;
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    
    // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
}

- (IBAction)stopButtonClicked:(id)sender {
  [[NSNotificationCenter defaultCenter] postNotificationName:kUICommandStopAt
                                                      object:self.idleEvent];
  [self close];
}

- (IBAction)splitButtonClicked:(id)sender {
  [[NSNotificationCenter defaultCenter] postNotificationName:kUICommandSplitAt
                                                      object:self.idleEvent];
  [self close];
}

- (IBAction)ignoreButtonClicked:(id)sender {
  [self close];
}


@end
