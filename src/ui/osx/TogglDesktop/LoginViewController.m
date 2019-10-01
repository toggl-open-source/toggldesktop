//
//  LoginViewController.m
//
//  Copyright (c) 2014 TogglDesktop developers. All rights reserved.
//

#import "LoginViewController.h"
#import "Utils.h"
#import "NSTextField+Ext.h"
#import "TogglDesktop-Swift.h"
#import "NSTextFieldClickablePointer.h"
#import "AutocompleteDataSource.h"
#import "NSCustomComboBox.h"

typedef NS_ENUM (NSUInteger, TabViewType)
{
	TabViewTypeLogin,
	TabViewTypeSingup,
};

static NSString *const emailMissingError = @"Please enter valid email address";
static NSString *const passwordMissingError = @"A password is required";
static NSString *const countryNotSelectedError = @"Please select Country before signing up";
static NSString *const tosAgreeError = @"You must agree to the terms of service and privacy policy to use Toggl";

typedef NS_ENUM (NSUInteger, UserAction)
{
	UserActionAccountLogin,
	UserActionAccountSignup,
	UserActionGoogleLogin,
	UserActionGoogleSignup,
};

@interface LoginViewController () <NSTextFieldDelegate, NSTableViewDataSource, NSComboBoxDataSource, NSComboBoxDelegate>
@property (weak) IBOutlet NSTabView *tabView;
@property (weak) IBOutlet NSTextField *email;
@property (weak) IBOutlet NSSecureTextField *password;
@property (weak) IBOutlet NSButton *loginGooglBtn;
@property (weak) IBOutlet NSTextFieldClickablePointer *forgotPasswordTextField;
@property (weak) IBOutlet NSTextFieldClickablePointer *signUpLink;
@property (weak) IBOutlet NSTextFieldClickablePointer *loginLink;
@property (weak) IBOutlet NSCustomComboBox *countrySelect;
@property (weak) IBOutlet FlatButton *tosCheckbox;
@property (weak) IBOutlet NSTextFieldClickablePointer *tosLink;
@property (weak) IBOutlet NSTextFieldClickablePointer *privacyLink;
@property (weak) IBOutlet FlatButton *loginButton;
@property (weak) IBOutlet FlatButton *signupButton;
@property (weak) IBOutlet NSBox *boxView;
@property (weak) IBOutlet NSProgressIndicator *loginLoaderView;
@property (weak) IBOutlet NSProgressIndicator *signUpLoaderView;
@property (weak) IBOutlet NSButton *signupGoogleBtn;

@property (nonatomic, strong) AutocompleteDataSource *countryAutocompleteDataSource;
@property (nonatomic, assign) BOOL countriesLoaded;
@property (nonatomic, assign) NSInteger selectedCountryID;
@property (nonatomic, assign) TabViewType currentTab;
@property (nonatomic, assign) UserAction userAction;

- (IBAction)clickLoginButton:(id)sender;
- (IBAction)clickSignupButton:(id)sender;
- (IBAction)countrySelected:(id)sender;

@end

@implementation LoginViewController
extern void *ctx;

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
	self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
	if (self)
	{
		self.countryAutocompleteDataSource = [[AutocompleteDataSource alloc] initWithNotificationName:kDisplayCountries];
	}
	return self;
}

- (void)viewDidLoad
{
	[super viewDidLoad];

	[self initCommon];
	[self initCountryAutocomplete];

	// Default
	[self changeTabView:TabViewTypeLogin];
}

- (void)initCommon
{
	self.signUpLink.delegate = self;
	self.loginLink.delegate = self;
	self.countrySelect.delegate = self;
	self.email.delegate = self;
	self.password.delegate = self;

	self.forgotPasswordTextField.titleUnderline = YES;
	self.signUpLink.titleUnderline = YES;
	self.tosLink.titleUnderline = YES;
	self.privacyLink.titleUnderline = YES;
	self.loginLink.titleUnderline = YES;

	self.boxView.wantsLayer = YES;
	self.boxView.layer.masksToBounds = NO;
	self.boxView.shadow = [[NSShadow alloc] init];
	self.boxView.layer.shadowColor = [NSColor colorWithWhite:0 alpha:0.1].CGColor;
	self.boxView.layer.shadowOpacity = 1.0;
	self.boxView.layer.shadowOffset = CGSizeMake(0, -2);
	self.boxView.layer.shadowRadius = 6;

	self.view.wantsLayer = YES;
	self.view.layer.backgroundColor = [NSColor colorWithPatternImage:[NSImage imageNamed:@"background-pattern"]].CGColor;

	self.userAction = UserActionAccountLogin;
}

- (void)initCountryAutocomplete {
	self.countryAutocompleteDataSource.combobox = self.countrySelect;
	self.countryAutocompleteDataSource.combobox.dataSource = self.countryAutocompleteDataSource;
	[self.countryAutocompleteDataSource setFilter:@""];
	self.countriesLoaded = NO;
	self.selectedCountryID = -1;
}

- (void)viewDidAppear
{
	[super viewDidAppear];

	// As we change the cursor's color to white
	// so, we have to reset cursor color
	[self.email resetCursorColor];
	[self.password resetCursorColor];

	[self.email.window makeFirstResponder:self.email];
	[self changeTabView:TabViewTypeLogin];
}

- (IBAction)clickLoginButton:(id)sender
{
	self.userAction = UserActionAccountLogin;

	// Validate all values inserted
	if (![self validateFormForAction:self.userAction])
	{
		return;
	}

	NSString *email = [self.email stringValue];
	NSString *pass = [self.password stringValue];

	[self.password setStringValue:@""];

	// for empty State
	[self setUserSignUp:NO];
	[self showLoaderView:YES];

	if (!toggl_login_async(ctx, [email UTF8String], [pass UTF8String]))
	{
		return;
	}
}

- (void)textFieldClicked:(id)sender
{
	if (sender == self.forgotPasswordTextField)
	{
		toggl_password_forgot(ctx);
		return;
	}

	if (sender == self.signUpLink)
	{
		[self changeTabView:TabViewTypeSingup];
		return;
	}

	if (sender == self.loginLink)
	{
		[self changeTabView:TabViewTypeLogin];
		return;
	}

	if (sender == self.tosLink)
	{
		toggl_tos(ctx);
		return;
	}

	if (sender == self.privacyLink)
	{
		toggl_privacy_policy(ctx);
		return;
	}
}

- (void)changeTabView:(TabViewType)type
{
	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kHideDisplayError
																object:nil];
	[self.tabView selectTabViewItemAtIndex:type];
	self.currentTab = type;

	// Focus on email when changing mode
	[self.view.window makeFirstResponder:self.email];

	switch (type)
	{
		case TabViewTypeLogin :
			// Update nextkeyView
			[self.password setNextKeyView:self.loginButton];
			[self.loginGooglBtn setNextKeyView:self.email];
			break;

		case TabViewTypeSingup :
			self.countrySelect.stringValue = @"";
			[self.countryAutocompleteDataSource setFilter:@""];
			if (!self.countriesLoaded)
			{
				// Load countries in signup view
				toggl_get_countries_async(ctx);
				self.countriesLoaded = YES;
			}

			// Update nextkeyView
			[self.password setNextKeyView:self.countrySelect];
			[self.signupButton setNextKeyView:self.email];
			break;
	}
}

- (void)startGoogleAuthentication
{
	__weak typeof(self) weakSelf = self;
	[GoogleAuthenticationServerHelper authorize:^(NSString *token, NSError *error) {
		 typeof(self) strongSelf = weakSelf;
		 if (error != nil)
		 {
			 [strongSelf handleGoogleError:error];
		 }
		 else if (token != nil)
		 {
			 [strongSelf handleGoogleToken:token];
		 }
	 }];
	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kHideDisplayError
																object:nil];
}

- (void)handleGoogleError:(NSError *)error
{
	NSString *errorStr = [error localizedDescription];

	NSLog(@"Login error: %@", errorStr);

	if ([errorStr isEqualToString:@"access_denied"])
	{
		errorStr = @"Google login access was denied to app.";
	}

	if ([errorStr isEqualToString:@"The operation couldn’t be completed. (com.google.GTMOAuth2 error -1000.)"])
	{
		errorStr = @"Window was closed before login completed.";
	}

	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kDisplayError
																object:errorStr];
}

- (void)handleGoogleToken:(NSString *)token
{
	[self showLoaderView:YES];
	switch (self.userAction)
	{
		case UserActionGoogleSignup :
			toggl_google_signup_async(ctx, [token UTF8String], self.selectedCountryID);
			break;
		case UserActionGoogleLogin :
			toggl_google_login_async(ctx, [token UTF8String]);
			break;
		default :
			break;
	}
}

- (BOOL)validateFormForAction:(UserAction)action
{
	switch (action)
	{
		case UserActionAccountLogin :
			if (![self isEmalValid])
			{
				return NO;
			}
			if (![self isPasswordValid])
			{
				return NO;
			}
			return YES;

		case UserActionAccountSignup :
			if (![self isEmalValid])
			{
				return NO;
			}
			if (![self isPasswordValid])
			{
				return NO;
			}
			if (![self isCountryValid])
			{
				return NO;
			}
			if (![self isTOSValid])
			{
				return NO;
			}
			return YES;

		case UserActionGoogleLogin :
			return YES;

		case UserActionGoogleSignup :
			if (![self isCountryValid])
			{
				return NO;
			}
			if (![self isTOSValid])
			{
				return NO;
			}
			return YES;

		default :
			break;
	}

	return NO;
}

- (IBAction)clickSignupButton:(id)sender
{
	self.userAction = UserActionAccountSignup;

	// Validate all values inserted
	if (![self validateFormForAction:self.userAction])
	{
		return;
	}

	NSString *email = [self.email stringValue];
	NSString *pass = [self.password stringValue];

	[self.password setStringValue:@""];

	// for empty State
	[self setUserSignUp:YES];
	// Show loader and disable text boxs
	[self showLoaderView:YES];

	if (!toggl_signup_async(ctx, [email UTF8String], [pass UTF8String], self.selectedCountryID))
	{
		return;
	}
}

- (IBAction)countrySelected:(id)sender
{
	NSString *key = self.countrySelect.stringValue;
	AutocompleteItem *item = [self.countryAutocompleteDataSource get:key];

	self.selectedCountryID = item.ID;
}

- (void)controlTextDidChange:(NSNotification *)aNotification
{
	if ([[aNotification object] isKindOfClass:[NSCustomComboBox class]])
	{
		NSCustomComboBox *box = [aNotification object];
		NSString *filter = [box stringValue];

		[self.countryAutocompleteDataSource setFilter:filter];

		// Hide dropdown if filter is empty or nothing was found
		if (!filter || ![filter length] || !self.countryAutocompleteDataSource.count)
		{
			if ([box isExpanded] == YES)
			{
				[box setExpanded:NO];
			}
			return;
		}

		if ([box isExpanded] == NO)
		{
			[box setExpanded:YES];
		}
	}
}

- (BOOL)control:(NSControl *)control textView:(NSTextView *)textView doCommandBySelector:(SEL)commandSelector {
	if (control == self.email || control == self.password)
	{
		if (commandSelector == @selector(insertNewline:))
		{
			switch (self.currentTab)
			{
				case TabViewTypeLogin :
					[self clickLoginButton:self.loginButton];
					break;
				case TabViewTypeSingup :
					[self clickSignupButton:self.signupButton];
					break;
			}
			return YES;
		}
	}
	return NO;
}

- (IBAction)loginGoogleOnTap:(id)sender
{
	// for empty State
	self.userAction = UserActionGoogleLogin;
	[self setUserSignUp:NO];
	[self startGoogleAuthentication];
}

- (void)setUserSignUp:(BOOL)isSignUp
{
	[[NSUserDefaults standardUserDefaults] setBool:isSignUp forKey:kUserHasBeenSignup];
	[[NSUserDefaults standardUserDefaults] synchronize];
}

- (void)showLoaderView:(BOOL)show
{
	self.loginLoaderView.hidden = !show;
	self.signUpLoaderView.hidden = !show;

	if (show)
	{
		[self.loginLoaderView startAnimation:self];
		[self.signUpLoaderView startAnimation:self];
	}
	else
	{
		[self.loginLoaderView stopAnimation:self];
		[self.signUpLoaderView stopAnimation:self];
	}

	self.email.enabled = !show;
	self.password.enabled = !show;
	self.loginButton.enabled = !show;
	self.signupButton.enabled = !show;
	self.loginGooglBtn.enabled = !show;
	self.loginLink.enabled = !show;
	self.signUpLink.enabled = !show;
}

- (void)resetLoader
{
	[self showLoaderView:NO];
}

- (IBAction)signupGoogleBtnOnTap:(id)sender
{
	self.userAction = UserActionGoogleSignup;
	if (![self validateFormForAction:self.userAction])
	{
		return;
	}

	[self setUserSignUp:NO];
	[self startGoogleAuthentication];
}

- (BOOL)isEmalValid
{
	NSString *email = [self.email stringValue];

	if (email == nil || !email.length)
	{
		[self.email.window makeFirstResponder:self.email];
		[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kDisplayError
																	object:emailMissingError];
		return NO;
	}
	return YES;
}

- (BOOL)isPasswordValid
{
	NSString *pass = [self.password stringValue];

	if (pass == nil || !pass.length)
	{
		[self.password.window makeFirstResponder:self.password];
		[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kDisplayError
																	object:passwordMissingError];
		return NO;
	}
	return YES;
}

- (BOOL)isCountryValid
{
	if (self.selectedCountryID == -1 || self.countrySelect.stringValue.length == 0)
	{
		[self.countrySelect.window makeFirstResponder:self.countrySelect];
		[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kDisplayError
																	object:countryNotSelectedError];
		return NO;
	}
	return YES;
}

- (BOOL)isTOSValid
{
	BOOL tosChecked = [Utils stateToBool:[self.tosCheckbox state]];

	if (!tosChecked)
	{
		[self.tosCheckbox.window makeFirstResponder:self.tosCheckbox];
		[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kDisplayError
																	object:tosAgreeError];
		return NO;
	}
	return YES;
}

@end
