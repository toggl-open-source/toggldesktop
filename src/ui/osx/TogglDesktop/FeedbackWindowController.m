//
//  FeedbackWindowController.m
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 29/01/2014.
//  Copyright (c) 2014 TogglDesktop developers. All rights reserved.
//

#import "FeedbackWindowController.h"
#import "toggl_api.h"

@implementation FeedbackWindowController

extern void *ctx;

- (IBAction)uploadImageClick:(id)sender
{
	NSOpenPanel *panel = [NSOpenPanel openPanel];

	[panel beginWithCompletionHandler:^(NSInteger result) {
		 if (result == NSFileHandlingPanelOKButton)
		 {
			 NSURL *url = [[panel URLs] objectAtIndex:0];
			 NSLog(@"Selected document: %@", url);
			 [self.selectedImageTextField setStringValue:[url path]];
			 [self.selectedImageTextField setHidden:NO];
		 }
	 }];
}

- (IBAction)sendClick:(id)sender
{
	if (self.topicComboBox.stringValue == nil
		|| [self.topicComboBox.stringValue isEqualToString:@""])
	{
		[self.topicComboBox becomeFirstResponder];
		return;
	}
	if (self.contentTextView.string == nil
		|| [self.contentTextView.string isEqualToString:@""])
	{
		[self.contentTextView becomeFirstResponder];
		return;
	}

	if (!toggl_feedback_send(ctx,
							 [self.topicComboBox.stringValue UTF8String],
							 [self.contentTextView.string UTF8String],
							 [self.selectedImageTextField.stringValue UTF8String]))
	{
		return;
	}

	[self.window close];
	[self.selectedImageTextField setStringValue:@""];
	[self.selectedImageTextField setHidden:YES];
	[self.contentTextView setString:@""];
	[self.topicComboBox setStringValue:@""];

	[[NSAlert alertWithMessageText:@"Thank you!"
					 defaultButton:nil
				   alternateButton:nil
					   otherButton:nil
		 informativeTextWithFormat:@"Your feedback was sent successfully."] runModal];
}

@end
