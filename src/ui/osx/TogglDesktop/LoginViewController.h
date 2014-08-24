//
//  LoginViewController.h
//  Toggl Desktop on the Mac
//
//  Created by Tambet Masik on 9/24/13.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "kopsik_api.h"
#import "NSTextFieldClickablePointer.h"

@interface LoginViewController : NSViewController
@property IBOutlet NSTextField *email;
@property IBOutlet NSTextField *password;
@property IBOutlet NSTextFieldClickablePointer *googleLoginTextField;
@property IBOutlet NSTextFieldClickablePointer *forgotPasswordTextField;
- (IBAction)clickLoginButton:(id)sender;
@end
