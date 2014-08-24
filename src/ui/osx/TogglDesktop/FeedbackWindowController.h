//
//  FeedbackWindowController.h
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 29/01/2014.
//  Copyright (c) 2014 TogglDesktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface FeedbackWindowController : NSWindowController
@property IBOutlet NSComboBox *topicComboBox;
@property IBOutlet NSTextView *contentTextView;
@property IBOutlet NSButton *uploadImageButton;
@property IBOutlet NSButton *sendButton;
@property IBOutlet NSTextField *selectedImageTextField;
- (IBAction)uploadImageClick:(id)sender;
- (IBAction)sendClick:(id)sender;
@end
