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
#import "NSTextFieldRoundedCorners.h"

@interface LoginViewController : NSViewController
@property IBOutlet NSTextFieldRoundedCorners *email;
@property IBOutlet NSTextFieldRoundedCorners *password;
@property IBOutlet NSTextFieldClickablePointer *googleLoginTextField;
@property IBOutlet NSTextFieldClickablePointer *forgotPasswordTextField;
- (IBAction)clickLoginButton:(id)sender;
- (IBAction)clickSignupButton:(id)sender;
@end
