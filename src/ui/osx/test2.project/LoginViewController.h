//
//  LoginViewController.h
//  kopsik_ui_osx
//
//  Created by Tambet Masik on 9/24/13.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "kopsik_api.h"
#import "NSTextFieldClickable.h"

@interface LoginViewController : NSViewController
@property (weak) IBOutlet NSTextField *email;
@property (weak) IBOutlet NSSecureTextField *password;
@property (weak) IBOutlet NSTextField *errorLabel;
@property (weak) IBOutlet NSTextFieldClickable *googleLoginTextField;
@property (weak) IBOutlet NSTextFieldClickable *passwordForgotTextField;
- (IBAction)clickLoginButton:(id)sender;
@end
