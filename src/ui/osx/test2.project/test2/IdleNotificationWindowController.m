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
@property IdleEvent *idleEvent;
@end

@implementation IdleNotificationWindowController

- (id)initWithWindow:(NSWindow *)window
{
    self = [super initWithWindow:window];
    if (self) {
      [[NSNotificationCenter defaultCenter] addObserver:self
                                               selector:@selector(eventHandler:)
                                                   name:kUIEventIdleFinished
                                                 object:nil];
    }
    return self;
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

-(void)eventHandler: (NSNotification *) notification
{
  if ([notification.name isEqualToString:kUIEventIdleFinished]) {
    self.idleEvent = notification.object;
    NSString *information = [NSString stringWithFormat:@"You have been idle for %d minutes.", self.idleEvent.seconds / 60];
    NSAssert(self.informationTextField != nil,
             @"self.informationTextField cannot be nil at this point");
    [self.informationTextField setStringValue:information];
    [self showWindow:self];
  }
}

@end
