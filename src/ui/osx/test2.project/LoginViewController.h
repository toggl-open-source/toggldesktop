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
@property (weak) IBOutlet NSTextField *email;
@property (weak) IBOutlet NSSecureTextField *password;
@property (weak) IBOutlet NSTextField *errorLabel;
@property (weak) IBOutlet NSTextFieldClickable *googleLoginTextField;
@property (weak) IBOutlet NSTextFieldClickable *passwordForgotTextField;
@property (weak) IBOutlet NSBox *troubleBox;
- (IBAction)clickLoginButton:(id)sender;
@end
