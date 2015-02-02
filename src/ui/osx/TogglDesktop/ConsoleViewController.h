//
//  ConsoleViewController.h
//  TogglDesktop
//
//  Copyright (c) 2014 Toggl Desktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface ConsoleViewController : NSWindowController {
}
@property (weak) IBOutlet NSTextField *entryTextField;
@property (weak) IBOutlet NSButton *runButton;
@property (unsafe_unretained) IBOutlet NSTextView *resultTextView;
- (IBAction)onRun:(id)sender;
@end
