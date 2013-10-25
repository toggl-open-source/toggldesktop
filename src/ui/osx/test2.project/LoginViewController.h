//
//  LoginViewController.h
//  kopsik_ui_osx
//
//  Created by Tambet Masik on 9/24/13.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "kopsik_api.h"

@interface LoginViewController : NSViewController
@property (weak) IBOutlet NSButton *loginWithGoogleButton;
@property (weak) IBOutlet NSTextField *email;
@property (weak) IBOutlet NSSecureTextField *password;
@property (weak) IBOutlet NSTextField *errorLabel;
@property (weak) IBOutlet NSButton *loginButton;
@property (weak) IBOutlet NSButton *proxyButton;
- (IBAction)clickLoginButton:(id)sender;
- (IBAction)clickGoogleLoginButton:(id)sender;
- (IBAction)clickProxySettingsButton:(id)sender;
@end
