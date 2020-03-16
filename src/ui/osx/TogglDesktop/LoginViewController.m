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
#import "TogglDesktop-Swift.h"

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
    UserActionAppleLogin,
    UserActionAppleSignup,
};

#define kLoginAppleViewTop 86.0
#define kLoginContainerHeight 440.0
#define kSignupAppleViewTop 166.0
#define kSignupContainerHeight 460.0

@interface LoginViewController () <NSTextFieldDelegate, NSTableViewDataSource, NSComboBoxDataSource, NSComboBoxDelegate, LoginSignupTouchBarDelegate, AppleAuthenticationServiceDelegate>
@property (weak) IBOutlet NSTextField *email;
@property (weak) IBOutlet NSSecureTextField *password;
@property (weak) IBOutlet NSTextFieldClickablePointer *forgotPasswordTextField;
@property (weak) IBOutlet NSTextFieldClickablePointer *signUpLink;
@property (weak) IBOutlet NSCustomComboBox *countrySelect;
@property (weak) IBOutlet FlatButton *tosCheckbox;
@property (weak) IBOutlet NSTextFieldClickablePointer *tosLink;
@property (weak) IBOutlet NSTextFieldClickablePointer *privacyLink;
@property (weak) IBOutlet NSBox *boxView;
@property (weak) IBOutlet NSProgressIndicator *loginLoaderView;

@property (weak) IBOutlet NSStackView *signUpGroupView;
@property (weak) IBOutlet NSLayoutConstraint *appleGoogleGroupViewTop;
@property (weak) IBOutlet NSLayoutConstraint *containerViewHeight;

@property (weak) IBOutlet NSButton *googleBtn;
@property (weak) IBOutlet NSButton *appleBtn;
@property (weak) IBOutlet FlatButton *userActionBtn;
@property (weak) IBOutlet NSTextField *donotHaveAccountLbl;

@property (nonatomic, strong) AutocompleteDataSource *countryAutocompleteDataSource;
@property (nonatomic, assign) NSInteger selectedCountryID;
@property (nonatomic, assign) TabViewType currentTab;
@property (nonatomic, assign) UserAction userAction;
@property (nonatomic, strong) LoginSignupTouchBar *loginTouchBar __OSX_AVAILABLE_STARTING(__MAC_10_12_2,__IPHONE_NA);

- (IBAction)userActionButtonOnClick:(id)sender;
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

    // Load countries in signup view
    toggl_get_countries_async(ctx);
}

- (void)initCommon
{
	self.signUpLink.delegate = self;
	self.countrySelect.delegate = self;
	self.email.delegate = self;
	self.password.delegate = self;

	self.forgotPasswordTextField.titleUnderline = YES;
	self.signUpLink.titleUnderline = YES;
	self.tosLink.titleUnderline = YES;
	self.privacyLink.titleUnderline = YES;

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

    if (@available(macOS 10.12.2, *))
    {
        self.loginTouchBar = [[LoginSignupTouchBar alloc] init];
        self.loginTouchBar.delegate = self;
    }

    #ifdef APP_STORE
    if (@available(macOS 10.15, *))
    {
        [AppleAuthenticationService shared].delegate = self;
        self.appleBtn.hidden = NO;
    } else {
        self.appleBtn.hidden = YES;
    }
    #else
        self.appleBtn.hidden = YES;
    #endif
}

- (void)initCountryAutocomplete {
	self.countryAutocompleteDataSource.combobox = self.countrySelect;
	self.countryAutocompleteDataSource.combobox.dataSource = self.countryAutocompleteDataSource;
	[self.countryAutocompleteDataSource setFilter:@""];
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

- (void)textFieldClicked:(id)sender
{
	if (sender == self.forgotPasswordTextField)
	{
		toggl_password_forgot(ctx);
		return;
	}

	if (sender == self.signUpLink)
	{
        TabViewType newType = self.currentTab == TabViewTypeSingup ? TabViewTypeLogin : TabViewTypeSingup;
		[self changeTabView:newType];
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
	self.currentTab = type;

	// Focus on email when changing mode
	[self.view.window makeFirstResponder:self.email];

	switch (type)
	{
		case TabViewTypeLogin :
            self.appleGoogleGroupViewTop.constant = kLoginAppleViewTop;
            self.containerViewHeight.constant = kLoginContainerHeight;
            self.signUpGroupView.hidden = YES;
            self.appleBtn.title = @" Log in with Apple";
            self.googleBtn.title = @" Log in with Google";
            self.userActionBtn.title = @"Login with email";
            self.forgotPasswordTextField.hidden = NO;
            self.signUpLink.stringValue = @"Sign up for free";
            self.donotHaveAccountLbl.hidden = NO;
            self.signUpLink.titleUnderline = YES;
			break;

		case TabViewTypeSingup :
            self.appleGoogleGroupViewTop.constant = kSignupAppleViewTop;
            self.containerViewHeight.constant = kSignupContainerHeight;
            self.signUpGroupView.hidden = NO;
            self.appleBtn.title = @" Sign up with Apple";
            self.googleBtn.title = @" Sign up with Google";
            self.userActionBtn.title = @"Sign up with email";
            self.forgotPasswordTextField.hidden = YES;
            self.signUpLink.stringValue = @"Back to Log in";
            self.donotHaveAccountLbl.hidden = YES;
            self.signUpLink.titleUnderline = YES;
			break;
	}

	// Reset touchbar
    if (@available(macOS 10.12.2, *))
    {
        self.touchBar = nil;
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
        case UserActionAppleLogin :
			return YES;

		case UserActionGoogleSignup :
        case UserActionAppleSignup :
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
            [self userActionButtonOnClick:control];
		}
	}
	return NO;
}

- (void)setUserSignUp:(BOOL)isSignUp
{
	[[NSUserDefaults standardUserDefaults] setBool:isSignUp forKey:kUserHasBeenSignup];
	[[NSUserDefaults standardUserDefaults] synchronize];
}

- (void)showLoaderView:(BOOL)show
{
	self.loginLoaderView.hidden = !show;
	if (show)
	{
		[self.loginLoaderView startAnimation:self];
	}
	else
	{
		[self.loginLoaderView stopAnimation:self];
	}

	self.email.enabled = !show;
	self.password.enabled = !show;
	self.userActionBtn.enabled = !show;
	self.signUpLink.enabled = !show;
    self.forgotPasswordTextField.enabled = !show;
}

- (void)resetLoader
{
	[self showLoaderView:NO];
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

- (NSTouchBar *)makeTouchBar
{
    if (@available(macOS 10.12.2, *))
    {
        switch (self.currentTab)
        {
            case TabViewTypeLogin :
                return [self.loginTouchBar makeTouchBarFor:LoginSignupModeLogin];

            case TabViewTypeSingup :
                return [self.loginTouchBar makeTouchBarFor:LoginSignupModeSignUp];
        }
    }
    return nil;
}

- (void)loginSignupTouchBarOn:(enum LoginSignupAction)action
{
	switch (action)
	{
		case LoginSignupActionLogin :
			[self clickLoginButton:self];
			break;
		case LoginSignupActionLoginGoogle :
			[self loginGoogleOnTap:self];
			break;
		case LoginSignupActionSignUp :
			[self clickSignupButton:self];
			break;
		case LoginSignupActionSignUpGoogle :
			[self signupGoogleBtnOnTap:self];
			break;
        case LoginSignupActionLoginApple:
            [self loginAppleBtnOnTap:self];
            break;
        case LoginSignupActionSignUpApple:
            [self signupAppleBtnOnTap:self];
		default :
			break;
	}
}

#pragma mark - User action

- (IBAction)userActionButtonOnClick:(id)sender
{
    switch (self.currentTab)
    {
        case TabViewTypeLogin :
            [self clickLoginButton:sender];
            break;
        case TabViewTypeSingup :
            [self clickSignupButton:sender];
            break;
    }
}

- (void)clickSignupButton:(id)sender
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
    [self showLoaderView:YES];

    if (!toggl_signup_async(ctx, [email UTF8String], [pass UTF8String], self.selectedCountryID))
    {
        return;
    }
}

- (void)clickLoginButton:(id)sender
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

- (IBAction)countrySelected:(id)sender
{
    NSString *key = self.countrySelect.stringValue;
    AutocompleteItem *item = [self.countryAutocompleteDataSource get:key];
    self.selectedCountryID = item.ID;
}

- (IBAction)appleBtnOnClick:(id)sender
{
    switch (self.currentTab)
    {
        case TabViewTypeLogin :
            [self loginAppleBtnOnTap:sender];
            break;
        case TabViewTypeSingup :
            [self signupAppleBtnOnTap:sender];
            break;
    }
}

- (IBAction)googleBtnOnClick:(id)sender
{
    switch (self.currentTab)
    {
        case TabViewTypeLogin :
            [self loginGoogleOnTap:sender];
            break;
        case TabViewTypeSingup :
            [self signupGoogleBtnOnTap:sender];
            break;
    }
}

- (void)signupGoogleBtnOnTap:(id)sender
{
    self.userAction = UserActionGoogleSignup;
    if (![self validateFormForAction:self.userAction])
    {
        return;
    }

    [self setUserSignUp:NO];
    [self startGoogleAuthentication];
}

- (void)loginGoogleOnTap:(id)sender
{
    self.userAction = UserActionGoogleLogin;
    [self setUserSignUp:NO];
    [self startGoogleAuthentication];
}

- (void)signupAppleBtnOnTap:(id)sender
{
    #ifdef APP_STORE
    if (@available(macOS 10.15, *))
    {
        // Validate all values inserted
        if (![self validateFormForAction:UserActionAppleSignup])
        {
            return;
        }

        self.userAction = UserActionAppleSignup;
        [self setUserSignUp:YES];
        [self showLoaderView:YES];
        [[AppleAuthenticationService shared] requestAuth];
    }
    #endif
}

- (void)loginAppleBtnOnTap:(id)sender
{
    #ifdef APP_STORE
    if (@available(macOS 10.15, *))
    {
        self.userAction = UserActionAppleLogin;
        [self setUserSignUp:NO];
        [self showLoaderView:YES];
        [[AppleAuthenticationService shared] requestAuth];
    }
    #endif
}

#pragma mark - AppleAuthenticationServiceDelegate

- (NSWindow *)appleAuthenticationPresentOnWindow
{
    return self.view.window;
}

- (void)appleAuthenticationDidCompleteWith:(NSString *)token fullName:(NSString *)fullName
{
    // Login or Signup
    switch (self.userAction)
    {
        case UserActionAppleSignup :
            toggl_apple_signup_async(ctx, [token UTF8String], self.selectedCountryID, [fullName UTF8String]);
            break;
        case UserActionAppleLogin :
            toggl_apple_login_async(ctx, [token UTF8String]);
            break;
        default :
            break;
    }
}

- (void)appleAuthenticationDidFailedWith:(NSError *)error
{
    [self showLoaderView:NO];

    // User cancel
    if (error.code == 1001) {
        return;
    }

    [[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kDisplayError
                                                                object:error.description];
}
@end
