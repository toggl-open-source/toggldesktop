//
//  BetterFocusAutoCompleteInput.m
//  TogglDesktop
//
//  Created by Nghia Tran on 3/7/19.
//  Copyright © 2019 Alari. All rights reserved.
//

#import "BetterFocusAutoCompleteInput.h"

@implementation BetterFocusAutoCompleteInput

- (BOOL)becomeFirstResponder
{
	[self.responderDelegate didBecomeFirstResponder:self];
	return [super becomeFirstResponder];
}

- (BOOL)resignFirstResponder
{
	[self.responderDelegate didResignFirstResponder:self];
	return [super resignFirstResponder];
}

- (void)mouseDown:(NSEvent *)event
{
	[super mouseDown:event];
	[self.responderDelegate didBecomeFirstResponder:self];
}

- (void)textDidBeginEditing:(NSNotification *)notification
{
	[super textDidBeginEditing:notification];
	[self.responderDelegate didBecomeFirstResponder:self];
}

- (void)textDidEndEditing:(NSNotification *)notification
{
	[super textDidEndEditing:notification];
	[self.responderDelegate didResignFirstResponder:self];
}

@end
