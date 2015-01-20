//
//  ConsoleViewController.m
//  TogglDesktop
//
//  Copyright (c) 2014 Toggl Desktop developers. All rights reserved.
//

#import "ConsoleViewController.h"

@interface ConsoleViewController ()

@end

@implementation ConsoleViewController

void *ctx;

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
	int err = luaL_loadstring(L, [self.entryTextField.stringValue UTF8String]);

	if (!err)
	{
		err = lua_pcall(L, 0, LUA_MULTRET, 0);
	}

	int argc = lua_gettop(L);
	[self appendToResultView:[NSString stringWithFormat:@"%d value(s) returned\n", argc]];
	for (int i = 0; i < argc; i++)
	{
		if (lua_isstring(L, -1))
		{
			[self appendToResultView:[NSString stringWithFormat:@"%s\n", lua_tostring(L, -1)]];
		}
		else if (lua_isnumber(L, -1))
		{
			[self appendToResultView:[NSString stringWithFormat:@"%ld\n", lua_tointeger(L, -1)]];
		}
		else if (lua_isboolean(L, -1))
		{
			[self appendToResultView:[NSString stringWithFormat:@"%d\n", lua_toboolean(L, -1)]];
		}
		else
		{
			[self appendToResultView:@"ok\n"];
		}
		lua_pop(L, -1);
	}
	[self appendToResultView:@"\n"];

	self.lastEntry = self.entryTextField.stringValue;
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
