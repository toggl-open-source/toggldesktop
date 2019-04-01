//
//  LoginViewController.m
//
//  Copyright (c) 2014 TogglDesktop developers. All rights reserved.
//

#import "LoginViewController.h"
#import "toggl_api.h"
#import "UIEvents.h"
#import "GTMOAuth2WindowController.h"
#import "Utils.h"
#import "UIEvents.h"
#import "const.h"
#import "NSTextField+Ext.h"
#import "TogglDesktop-Swift.h"

typedef NS_ENUM (NSUInteger, TabViewType)
{
	TabViewTypeLogin,
	TabViewTypeSingup,
};

@interface LoginViewController ()
@property AutocompleteDataSource *countryAutocompleteDataSource;
@property (weak) IBOutlet NSTabView *tabView;
@property (weak) IBOutlet NSTextField *email;
@property (weak) IBOutlet NSSecureTextField *password;
@property (weak) IBOutlet NSButton *loginGooglBtn;
@property (weak) IBOutlet NSTextFieldClickablePointer *forgotPasswordTextField;
@property (weak) IBOutlet NSTextFieldClickablePointer *signUpLink;
@property (weak) IBOutlet NSTextFieldClickablePointer *loginLink;
@property (weak) IBOutlet NSCustomComboBox *countrySelect;

@property BOOL countriesLoaded;
@property uint64_t selectedCountryID;
@property (weak) IBOutlet FlatButton *tosCheckbox;
@property (weak) IBOutlet NSTextFieldClickablePointer *tosLink;
@property (weak) IBOutlet NSTextFieldClickablePointer *privacyLink;
@property (weak) IBOutlet FlatButton *loginButton;
@property (weak) IBOutlet FlatButton *signupButton;
@property (assign, nonatomic) TabViewType currentTab;
@property (weak) IBOutlet NSBox *boxView;

- (IBAction)clickLoginButton:(id)sender;
- (IBAction)clickSignupButton:(id)sender;
- (IBAction)countrySelected:(id)sender;

@end

@implementation LoginViewController
NSString *emailMissingError = @"Please enter valid email address";
NSString *passwordMissingError = @"A password is required";
NSString *countryNotSelectedError = @"Please select Country before signing up";
NSString *tosAgreeError = @"You must agree to the terms of service and privacy policy to use Toggl";
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
	// Validate all values inserted
	if (![self validateForm:NO])
	{
		return;
	}

	NSString *email = [self.email stringValue];
	NSString *pass = [self.password stringValue];

	[self.password setStringValue:@""];

	// for empty State
	[self setUserSignUp:NO];

	if (!toggl_login(ctx, [email UTF8String], [pass UTF8String]))
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
				toggl_get_countries(ctx);
				self.countriesLoaded = YES;
			}

			// Update nextkeyView
			[self.password setNextKeyView:self.countrySelect];
			[self.signupButton setNextKeyView:self.email];
			break;
	}
}

- (void)startGoogleLogin
{
	NSString *scope = @"profile email";
	NSString *clientID = @"426090949585-uj7lka2mtanjgd7j9i6c4ik091rcv6n5.apps.googleusercontent.com";
	// According to Google docs, in installed apps the client secret is not expected to stay secret:
	NSString *clientSecret = @"6IHWKIfTAMF7cPJsBvoGxYui";

	GTMOAuth2WindowController *windowController;

	windowController = [[GTMOAuth2WindowController alloc] initWithScope:scope
															   clientID:clientID
														   clientSecret:clientSecret
													   keychainItemName:nil
														 resourceBundle:nil];

	[windowController signInSheetModalForWindow:[[NSApplication sharedApplication] mainWindow]
									   delegate:self
							   finishedSelector:@selector(viewController:finishedWithAuth:error:)];
	[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kHideDisplayError
																object:nil];
}

- (void)viewController:(GTMOAuth2WindowController *)viewController
	  finishedWithAuth:(GTMOAuth2Authentication *)auth
				 error:(NSError *)error
{
	if (error != nil)
	{
		NSString *errorStr = [error localizedDescription];

		NSData *responseData = [[error userInfo] objectForKey:kGTMHTTPFetcherStatusDataKey];
		if ([responseData length] > 0)
		{
			// Show the body of the server's authentication failure response
			errorStr = [[NSString alloc] initWithData:responseData
											 encoding:NSUTF8StringEncoding];
		}
		else
		{
			NSString *str = [[error userInfo] objectForKey:kGTMOAuth2ErrorMessageKey];
			if (str != nil)
			{
				if ([str length] > 0)
				{
					errorStr = str;
				}
			}
		}

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
		return;
	}

	toggl_google_login(ctx, [auth.accessToken UTF8String]);
}

- (BOOL)validateForm:(BOOL)signup
{
	// check if email is inserted
	NSString *email = [self.email stringValue];

	if (email == nil || !email.length)
	{
		[self.email.window makeFirstResponder:self.email];
		[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kDisplayError
																	object:emailMissingError];
		return NO;
	}

	// check if password is inserted
	NSString *pass = [self.password stringValue];

	if (pass == nil || !pass.length)
	{
		[self.password.window makeFirstResponder:self.password];
		[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kDisplayError
																	object:passwordMissingError];
		return NO;
	}

	if (!signup)
	{
		return YES;
	}

	// check if country is selected
	if (self.selectedCountryID == -1 || self.countrySelect.stringValue.length == 0)
	{
		[self.countrySelect.window makeFirstResponder:self.countrySelect];
		[[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kDisplayError
																	object:countryNotSelectedError];
		return NO;
	}

	// check if tos and privacy policy is checked
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

- (IBAction)clickSignupButton:(id)sender
{
	// Validate all values inserted
	if (![self validateForm:YES])
	{
		return;
	}

	NSString *email = [self.email stringValue];
	NSString *pass = [self.password stringValue];

	[self.password setStringValue:@""];

	// for empty State
	[self setUserSignUp:YES];

	if (!toggl_signup(ctx, [email UTF8String], [pass UTF8String], self.selectedCountryID))
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

- (IBAction)loginGoogleOnTap:(id)sender {
	// for empty State
	[self setUserSignUp:NO];
	[self startGoogleLogin];
}

- (void)setUserSignUp:(BOOL)isSignUp
{
	[[NSUserDefaults standardUserDefaults] setBool:isSignUp forKey:kUserHasBeenSignup];
	[[NSUserDefaults standardUserDefaults] synchronize];
}

@end
