//
//  PreferencesWindowController.h
//  Kopsik
//
//  Created by Tanel Lebedev on 22/10/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface PreferencesWindowController : NSWindowController
@property (weak) IBOutlet NSTextField *hostTextField;
@property (weak) IBOutlet NSTextField *portTextField;
@property (weak) IBOutlet NSTextField *usernameTextField;
@property (weak) IBOutlet NSTextField *passwordTextField;
@property (weak) IBOutlet NSButton *useProxyButton;
@property (weak) IBOutlet NSButton *useIdleDetectionButton;
- (IBAction)useProxyButtonChanged:(id)sender;
- (IBAction)hostTextFieldChanged:(id)sender;
- (IBAction)portTextFieldChanged:(id)sender;
- (IBAction)usernameTextFieldChanged:(id)sender;
- (IBAction)passwordTextFieldChanged:(id)sender;
- (IBAction)useIdleDetectionButtonChanged:(id)sender;
@end
