//
//  IdleNotificationWindowController.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 06/11/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "IdleNotificationWindowController.h"
#import "UIEvents.h"
#import "IdleEvent.h"

@interface IdleNotificationWindowController ()
@property IdleEvent *idleEvent;
@end

@implementation IdleNotificationWindowController

- (id)initWithWindow:(NSWindow *)window {
    self = [super initWithWindow:window];
    if (self) {
      [[NSNotificationCenter defaultCenter] addObserver:self
                                               selector:@selector(eventHandler:)
                                                   name:kUIEventIdleFinished
                                                 object:nil];
    }
    return self;
}

- (void)windowDidLoad {
  [super windowDidLoad];
  
  [self renderIdle];
}

- (void)renderIdle {
  NSLog(@"IdleNotificationWindowController windowDidLoad");
  NSDateFormatter *format = [[NSDateFormatter alloc] init];
  [format setDateFormat:@"HH:mm:ss"];
  NSString *dateString = [format stringFromDate:self.idleEvent.started];
  
  NSString *idleSince = [NSString stringWithFormat:@"You have been idle since %@", dateString];
  [self.idleSinceTextField setStringValue:idleSince];

  NSString *idleAmount = [NSString stringWithFormat:@" (%ld minutes)", self.idleEvent.seconds / 60];
  [self.idleAmountTextField setStringValue:idleAmount];

}

- (IBAction)stopButtonClicked:(id)sender {
  [[NSNotificationCenter defaultCenter] postNotificationName:kUICommandStopAt
                                                      object:self.idleEvent];
  [self.window orderOut:nil];
}

- (IBAction)ignoreButtonClicked:(id)sender {
  [self.window orderOut:nil];
}

-(void)eventHandler: (NSNotification *) notification {
  if ([notification.name isEqualToString:kUIEventIdleFinished]) {
    self.idleEvent = notification.object;
    [self.window makeKeyAndOrderFront:nil];
    [self renderIdle];
  }
}

@end
