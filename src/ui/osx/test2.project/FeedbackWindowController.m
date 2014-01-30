//
//  FeedbackWindowController.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 29/01/2014.
//  Copyright (c) 2014 kopsik developers. All rights reserved.
//

#import "FeedbackWindowController.h"
#import "kopsik_api.h"
#import "Context.h"
#import "ErrorHandler.h"

@interface FeedbackWindowController ()

@end

@implementation FeedbackWindowController

- (id)initWithWindow:(NSWindow *)window {
    self = [super initWithWindow:window];
    if (self) {
      [[NSNotificationCenter defaultCenter] addObserver:self
                                               selector:@selector(eventHandler:)
                                                   name:kUIStateFeedbackSent
                                                 object:nil];
    }
    return self;
}

- (void)windowDidLoad {
    [super windowDidLoad];
    
    // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
}

- (void)eventHandler: (NSNotification *) notification {
  if ([notification.name isEqualToString:kUIStateFeedbackSent]) {
    [self performSelectorOnMainThread:@selector(feedbackSent) withObject:nil waitUntilDone:NO];
  }
}

- (void)feedbackSent {
  [self.window close];
  [self.selectedImageTextField setStringValue:@""];
  [self.selectedImageTextField setHidden:YES];
  [self.contentTextView setString:@""];
  [self.topicComboBox setStringValue:@""];
}

- (IBAction)uploadImageClick:(id)sender {
  NSOpenPanel* panel = [NSOpenPanel openPanel];
  [panel beginWithCompletionHandler:^(NSInteger result){
    if (result == NSFileHandlingPanelOKButton) {
      NSURL *url = [[panel URLs] objectAtIndex:0];
      NSLog(@"Selected document: %@", url);
      [self.selectedImageTextField setStringValue:[url path]];
      [self.selectedImageTextField setHidden:NO];
    }
  }];
}

- (IBAction)sendClick:(id)sender {
  if (self.topicComboBox.stringValue == nil
      || [self.topicComboBox.stringValue isEqualToString:@""]) {
    [self.topicComboBox becomeFirstResponder];
    return;
  }
  if (self.contentTextView.string == nil
      || [self.contentTextView.string isEqualToString:@""]) {
    [self.contentTextView becomeFirstResponder];
    return;
  }
  kopsik_feedback_send(ctx,
                       [self.topicComboBox.stringValue UTF8String],
                       [self.contentTextView.string UTF8String],
                       [self.selectedImageTextField.stringValue UTF8String],
                       on_send_result);
}

void on_send_result(kopsik_api_result res,
                    const char *errmsg) {
  if (KOPSIK_API_SUCCESS != res) {
    handle_error(res, errmsg);
    return;
  }
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateFeedbackSent
                                                      object:nil];
}

@end
