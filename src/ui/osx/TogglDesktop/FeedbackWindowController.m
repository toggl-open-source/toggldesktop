//
//  FeedbackWindowController.m
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 29/01/2014.
//  Copyright (c) 2014 TogglDesktop developers. All rights reserved.
//

#import "FeedbackWindowController.h"
#import "toggl_api.h"
#import "NSAlert+Utils.h"

@implementation FeedbackWindowController

extern void *ctx;

- (IBAction)uploadImageClick:(id)sender
{
	NSOpenPanel *panel = [NSOpenPanel openPanel];

	[panel beginWithCompletionHandler:^(NSInteger result) {
		 if (result == NSFileHandlingPanelOKButton)
		 {
			 NSURL *url = [[panel URLs] objectAtIndex:0];
			 self.selectedImageTextField.toolTip = [url path];
			 [self.selectedImageTextField setStringValue:[url lastPathComponent]];
			 [self.selectedImageTextField setHidden:NO];
		 }
	 }];
}

- (IBAction)sendClick:(id)sender
{
	if (self.topicComboBox.stringValue == nil
		|| [self.topicComboBox.stringValue isEqualToString:@""])
	{
		[[NSAlert alertWithMessageText:@"Feedback not sent!"
			 informativeTextWithFormat:@"Please choose a topic before sending feedback."] runModal];
		[self.topicComboBox.window makeFirstResponder:self.topicComboBox];
		return;
	}
	if (self.contentTextView.string == nil
		|| [self.contentTextView.string isEqualToString:@""])
	{
		[[NSAlert alertWithMessageText:@"Feedback not sent!"
			 informativeTextWithFormat:@"Please type in your feedback before sending."] runModal];
		[self.contentTextView.window makeFirstResponder:self.contentTextView];
		return;
	}

	if (!toggl_feedback_send(ctx,
							 [self.topicComboBox.stringValue UTF8String],
							 [self.contentTextView.string UTF8String],
							 [self.selectedImageTextField.toolTip UTF8String]))
	{
		[[NSAlert alertWithMessageText:@"Feedback not sent!"
			 informativeTextWithFormat:@"Please check that file you are sending is not larger than 5MB."] runModal];
		return;
	}

	[self.window close];
	[self.selectedImageTextField setStringValue:@""];
	[self.contentTextView setString:@""];
	[self.topicComboBox setStringValue:@""];

	[[NSAlert alertWithMessageText:@"Thank you!"
		 informativeTextWithFormat:@"Your feedback was sent successfully."] runModal];
}

@end
