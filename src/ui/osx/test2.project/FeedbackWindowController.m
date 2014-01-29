//
//  FeedbackWindowController.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 29/01/2014.
//  Copyright (c) 2014 kopsik developers. All rights reserved.
//

#import "FeedbackWindowController.h"

@interface FeedbackWindowController ()

@end

@implementation FeedbackWindowController

- (id)initWithWindow:(NSWindow *)window {
    self = [super initWithWindow:window];
    if (self) {
        // Initialization code here.
    }
    return self;
}

- (void)windowDidLoad {
    [super windowDidLoad];
    
    // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
}

- (IBAction)uploadImageClick:(id)sender {
  NSOpenPanel* panel = [NSOpenPanel openPanel];
  [panel beginWithCompletionHandler:^(NSInteger result){
    if (result == NSFileHandlingPanelOKButton) {
      NSURL*  theDoc = [[panel URLs] objectAtIndex:0];
      NSLog(@"Selected document: %@", theDoc);
      [self.selectedImageTextField setStringValue:theDoc.description];
      [self.selectedImageTextField setHidden:NO];
    }
  }];
}

- (IBAction)sendClick:(id)sender {
  [self.window close];
  [self.selectedImageTextField setStringValue:@""];
  [self.selectedImageTextField setHidden:YES];
  [self.contentTextView setString:@""];
  [self.topicComboBox setStringValue:@""];
}

@end
