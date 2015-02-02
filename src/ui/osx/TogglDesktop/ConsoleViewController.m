//
//  ConsoleViewController.m
//  TogglDesktop
//
//  Copyright (c) 2014 Toggl Desktop developers. All rights reserved.
//

#import "ConsoleViewController.h"

#import "Utils.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "toggl_api_lua.h"

@interface ConsoleViewController ()
@end

@implementation ConsoleViewController

void *ctx;
lua_State *L;

- (void)windowDidLoad
{
	[super windowDidLoad];

	L = luaL_newstate();
	luaL_openlibs(L);

	toggl_register_lua(ctx, L);

	lua_settop(L, 0);

	[self.entryTextField becomeFirstResponder];
}

- (IBAction)onRun:(id)sender
{
	NSString *result = [Utils runScript:self.entryTextField.stringValue withState:L];

	[self appendToResultView:result];

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
