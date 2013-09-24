//
//  LoginViewController.h
//  kopsik_ui_osx
//
//  Created by Tambet Masik on 9/24/13.
//  Copyright (c) 2013 Alari. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface LoginViewController : NSViewController
@property (weak) IBOutlet NSTextField *email;
@property (weak) IBOutlet NSSecureTextField *password;
@property (weak) IBOutlet NSTextField *errorLabel;
@property (weak) IBOutlet NSButton *loginButton;
@property (nonatomic, assign) SEL loginSuccess;
- (IBAction)clickLoginButton:(id)sender;

@end
