//
//  ConsoleViewController.h
//  TogglDesktop
//
//  Copyright (c) 2014 Toggl Desktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "toggl_api_lua.h"

@interface ConsoleViewController : NSWindowController {
	lua_State *L;
}
@property (weak) IBOutlet NSTextField *entryTextField;
@property (weak) IBOutlet NSButton *runButton;
@property (unsafe_unretained) IBOutlet NSTextView *resultTextView;
@property NSString *lastEntry;
- (IBAction)onRun:(id)sender;
@end
