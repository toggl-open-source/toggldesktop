//
//  LoginViewController.h
//  Toggl Desktop on the Mac
//
//  Created by Tambet Masik on 9/24/13.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "toggl_api.h"
#import "NSTextFieldClickablePointer.h"

@interface LoginViewController : NSViewController
@property (strong) IBOutlet NSBox *signUpBox;
@property (strong) IBOutlet NSBox *loginBox;
@property IBOutlet NSTextField *email;
@property IBOutlet NSTextField *password;
@property IBOutlet NSTextFieldClickablePointer *googleLoginTextField;
@property IBOutlet NSTextFieldClickablePointer *forgotPasswordTextField;
@property (strong) IBOutlet NSTextFieldClickablePointer *signUpLink;
@property (strong) IBOutlet NSTextFieldClickablePointer *loginLink;
- (IBAction)clickLoginButton:(id)sender;
- (IBAction)clickSignupButton:(id)sender;
@end
