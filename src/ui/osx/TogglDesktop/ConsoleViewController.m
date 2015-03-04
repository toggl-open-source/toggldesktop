//
//  ConsoleViewController.m
//  TogglDesktop
//
//  Copyright (c) 2014 Toggl Desktop developers. All rights reserved.
//

#import "ConsoleViewController.h"

#import "Utils.h"

@interface ConsoleViewController ()
@end

@implementation ConsoleViewController

void *ctx;

- (void)windowDidLoad
{
	[super windowDidLoad];

	[self.entryTextField becomeFirstResponder];
}

- (IBAction)onRun:(id)sender
{
	ScriptResult *result = [Utils runScript:self.entryTextField.stringValue];

	[self appendToResultView:result.text];

	self.entryTextField.stringValue = @"";
}

- (void)appendToResultView:(NSString *)text
{
	dispatch_async(dispatch_get_main_queue(), ^{
					   NSAttributedString *attr = [[NSAttributedString alloc] initWithString:text];

					   [[self.resultTextView textStorage] appendAttributedString:attr];
					   [self.resultTextView scrollRangeToVisible:NSMakeRange([[self.resultTextView string] length], 0)];
				   });
}

@end
