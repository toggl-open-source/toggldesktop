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
#import "AutocompleteDataSource.h"
#import "NSCustomComboBox.h"

@interface LoginViewController : NSViewController <NSTextFieldDelegate, NSTableViewDataSource, NSComboBoxDataSource, NSComboBoxDelegate>
@property (strong) IBOutlet NSBox *signUpBox;
@property (strong) IBOutlet NSBox *loginBox;
@property IBOutlet NSTextField *email;
@property IBOutlet NSTextField *password;
@property IBOutlet NSTextFieldClickablePointer *googleLoginTextField;
@property IBOutlet NSTextFieldClickablePointer *forgotPasswordTextField;
@property (strong) IBOutlet NSTextFieldClickablePointer *signUpLink;
@property (strong) IBOutlet NSTextFieldClickablePointer *loginLink;
@property (strong) IBOutlet NSCustomComboBox *countrySelect;
@property BOOL countriesLoaded;
@property uint64_t selectedCountryID;
- (IBAction)clickLoginButton:(id)sender;
- (IBAction)clickSignupButton:(id)sender;
- (IBAction)countrySelected:(id)sender;
@property (strong) IBOutlet NSButton *tosCheckbox;
@property (strong) IBOutlet NSTextFieldClickablePointer *tosLink;
@property (strong) IBOutlet NSTextFieldClickablePointer *privacyLink;
@property (strong) IBOutlet NSButton *loginButton;
@property (strong) IBOutlet NSButton *SignupButton;
@end
