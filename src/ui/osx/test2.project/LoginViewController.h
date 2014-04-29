//
//  LoginViewController.h
//  kopsik_ui_osx
//
//  Created by Tambet Masik on 9/24/13.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "kopsik_api.h"
#import "NSTextFieldClickable.h"

@interface LoginViewController : NSViewController
@property IBOutlet NSTextField *email;
@property IBOutlet NSTextField *password;
@property IBOutlet NSTextFieldClickable *googleLoginTextField;
@property IBOutlet NSTextFieldClickable *passwordForgotTextField;
- (IBAction)clickLoginButton:(id)sender;
@end
