//
//  LoginViewController.m
//
//  Copyright (c) 2014 TogglDesktop developers. All rights reserved.
//

#import "LoginViewController.h"
#import "Utils.h"
#import "NSTextField+Ext.h"
#import "Toggl_Track-Swift.h"
#import "NSTextFieldClickablePointer.h"
#import "AutocompleteDataSource.h"
#import "NSCustomComboBox.h"
#import "const.h"
#import "DesktopLibraryBridge.h"

typedef NS_ENUM (NSUInteger, TabViewType)
{
    TabViewTypeLogin,
    TabViewTypeSingup,
    TabViewTypeContinueSignin,
    TabViewTypeEmailInputSSO,
    TabViewTypeEmailExistsSSO
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
    UserActionContinueSSO
};

#define kLoginAppleViewTop 77.0f
#define kSSOLoginAppleViewTop 118.0f
#define kLoginContainerHeight 480.0
#define kSignupAppleViewTop 166.0
#define kSignupContainerHeight 480.0
#define kPasswordStrengthViewBottom -4
#define kPasswordStrengthViewCenterX 0

@interface LoginViewController () <NSTextFieldDelegate, NSTableViewDataSource, NSComboBoxDataSource, NSComboBoxDelegate, LoginSignupTouchBarDelegate, AppleAuthenticationServiceDelegate>
@property (weak) IBOutlet NSTextField *email;
@property (weak) IBOutlet NSSecureTextField *password;
@property (weak) IBOutlet NSTextFieldClickablePointer *forgotPasswordTextField;
@property (weak) IBOutlet NSTextFieldClickablePointer *signUpLink;
@property (weak) IBOutlet NSCustomComboBox *countrySelect;
@property (weak) IBOutlet FlatButton *tosCheckbox;
@property (weak) IBOutlet NSTextFieldClickablePointer *tosLink;
@property (weak) IBOutlet NSTextFieldClickablePointer *tosContinueLink;
@property (weak) IBOutlet NSTextFieldClickablePointer *privacyLink;
@property (weak) IBOutlet NSTextFieldClickablePointer *privacyContinueLink;
@property (weak) IBOutlet NSBox *boxView;
@property (weak) IBOutlet NSProgressIndicator *loginLoaderView;

@property (weak) IBOutlet NSStackView *signUpGroupView;
@property (weak) IBOutlet NSLayoutConstraint *appleGoogleGroupViewTop;
@property (weak) IBOutlet NSLayoutConstraint *containerViewHeight;

@property (weak) IBOutlet NSButton *googleBtn;
@property (weak) IBOutlet NSButton *appleBtn;
@property (weak) IBOutlet FlatButton *userActionBtn;
@property (weak) IBOutlet NSTextField *donotHaveAccountLbl;

@property (weak) IBOutlet NSTextField *welcomeToTogglLbl;
@property (weak) IBOutlet NSTextField *subWelcomeLbl;
@property (weak) IBOutlet NSStackView *socialButtonStackView;
@property (weak) IBOutlet NSTextField *orLbl;
@property (weak) IBOutlet NSBox *leftSeperatorLine;
@property (weak) IBOutlet NSBox *rightSeperatorLine;
@property (weak) IBOutlet NSLayoutConstraint *countryStackViewTop;
@property (weak) IBOutlet NSView *termSignUpContainerView;
@property (weak) IBOutlet NSView *termContinueSignInView;
@property (weak) IBOutlet NSTextFieldClickablePointer *loginWithSSOBtn;

@property (nonatomic, strong) PasswordStrengthView *passwordStrengthView;
@property (nonatomic, strong) AutocompleteDataSource *countryAutocompleteDataSource;
@property (nonatomic, assign) NSInteger selectedCountryID;
@property (nonatomic, assign) TabViewType currentTab;
@property (nonatomic, assign) UserAction userAction;
@property (nonatomic, strong) LoginSignupTouchBar *loginTouchBar __OSX_AVAILABLE_STARTING(__MAC_10_12_2,__IPHONE_NA);
@property (nonatomic, copy) NSString *token;
@property (nonatomic, copy) NSString *fullName;
@property (nonatomic, assign) BOOL isCountryLoaded;

@property (weak) IBOutlet NSTabView *tabView;
@property (weak) IBOutlet NSTabViewItem *loginAndSignUpTabItem;
@property (weak) IBOutlet NSTabViewItem *inputSSOTabItem;
@property (weak) IBOutlet NSTabViewItem *emailExistSSOTabItem;

// SSO
@property (weak) IBOutlet NSTextFieldClickablePointer *loginWithDifferentMethodBtn;
@property (weak) IBOutlet NSTextFieldClickablePointer *ssoCancelAndGoBackBtn;
@property (weak) IBOutlet NSTextField *emailSSOTextField;
@property (weak) IBOutlet NSTextFieldClickablePointer *backToSSOBtn;
@property (weak) IBOutlet NSTextField *ssoTitleLbl;
@property (assign, nonatomic) BOOL isLoginSignUpAsSSO;
@property (strong, nonatomic) SSOPayload *ssoPayload;

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
}

- (void)initCommon
{
    self.signUpLink.delegate = self;
    self.countrySelect.delegate = self;
    self.email.delegate = self;
    self.password.delegate = self;
    self.isCountryLoaded = NO;
    self.loginWithSSOBtn.delegate = self;
    self.loginWithDifferentMethodBtn.delegate = self;
    self.backToSSOBtn.delegate = self;
    self.emailSSOTextField.delegate = self;
    self.isLoginSignUpAsSSO = NO;

    self.backToSSOBtn.titleUnderline = YES;
    self.loginWithDifferentMethodBtn.titleUnderline = YES;
    self.forgotPasswordTextField.titleUnderline = YES;
    self.signUpLink.titleUnderline = YES;
    self.loginWithSSOBtn.titleUnderline = YES;
    self.tosLink.titleUnderline = YES;
    self.privacyLink.titleUnderline = YES;
    self.tosContinueLink.titleUnderline = YES;
    self.privacyContinueLink.titleUnderline = YES;
    self.tosContinueLink.delegate = self;
    self.privacyContinueLink.delegate = self;
    self.ssoCancelAndGoBackBtn.titleUnderline = YES;

    self.boxView.wantsLayer = YES;
    self.boxView.layer.masksToBounds = NO;
    self.boxView.shadow = [[NSShadow alloc] init];
    self.boxView.layer.shadowColor = [NSColor shadow].CGColor;
    self.boxView.layer.shadowOpacity = 1.0;
    self.boxView.layer.shadowOffset = CGSizeMake(0, -2);
    self.boxView.layer.shadowRadius = 6;

    self.view.wantsLayer = YES;
    self.view.layer.backgroundColor = NSColor.collectionViewBackgroundColor.CGColor;

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

    self.passwordStrengthView = [[PasswordStrengthView alloc] initWithNibName:@"PasswordStrengthView" bundle:nil];
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
        switch (self.currentTab) {
            case TabViewTypeLogin:
                [self changeTabView:TabViewTypeSingup];
                break;
            case TabViewTypeSingup:
                [self changeTabView:TabViewTypeLogin];
                break;
            case TabViewTypeContinueSignin:
                [self changeTabView:TabViewTypeLogin];
                break;
            case TabViewTypeEmailInputSSO:
                break;
            case TabViewTypeEmailExistsSSO:
                break;
        }
        return;
    }

    if (sender == self.tosLink || sender == self.tosContinueLink)
    {
        toggl_tos(ctx);
        return;
    }

    if (sender == self.privacyLink || sender == self.privacyContinueLink)
    {
        toggl_privacy_policy(ctx);
        return;
    }

    if (sender == self.loginWithSSOBtn) {
        [self changeTabView:TabViewTypeEmailInputSSO];
        return;
    }

    if (sender == self.loginWithDifferentMethodBtn) {
        [self changeTabView:TabViewTypeLogin];
        return;
    }

    if (sender == self.backToSSOBtn) {
        // As we go back to the main Login view
        // We reset all SSO states from the UI and Library
        [self setShouldEnableSSOAfterLogin:NO];
        [self changeTabView:TabViewTypeEmailInputSSO];
        return;
    }

    if (sender == self.ssoCancelAndGoBackBtn) {
        [self setShouldEnableSSOAfterLogin:NO];
        [self changeTabView:TabViewTypeLogin];
        return;
    }
}

- (void)changeTabView:(TabViewType)type
{
    [self changeTabView:type hideErrorMessage:YES];
}

- (void)changeTabView:(TabViewType)type hideErrorMessage:(BOOL)hideErrorMessage
{
    if (hideErrorMessage) {
        [[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kHideDisplayError
                                                                    object:nil];
    }

    self.currentTab = type;
    [self showLoaderView:NO];
    [self displayPasswordStrengthView:NO];
    [self loadCountryListIfNeed];

    switch (type)
    {
        case TabViewTypeLogin :
            [self.tabView selectTabViewItem:self.loginAndSignUpTabItem];
            self.appleGoogleGroupViewTop.constant = kLoginAppleViewTop;
            self.containerViewHeight.constant = kLoginContainerHeight;
            self.signUpGroupView.hidden = YES;
            self.appleBtn.title = @" Sign in with Apple";
            self.googleBtn.title = @" Sign in with Google";
            self.userActionBtn.title = @"Log in with email";
            self.forgotPasswordTextField.hidden = NO;
            self.signUpLink.stringValue = @"Sign up for free";
            self.donotHaveAccountLbl.hidden = NO;
            self.signUpLink.titleUnderline = YES;
            self.welcomeToTogglLbl.hidden = YES;

            self.loginWithSSOBtn.hidden = NO;
            self.subWelcomeLbl.hidden = YES;
            self.socialButtonStackView.hidden = NO;
            self.email.hidden = NO;
            self.password.hidden = NO;
            self.leftSeperatorLine.hidden = NO;
            self.rightSeperatorLine.hidden = NO;
            self.orLbl.hidden = NO;
            self.signUpLink.hidden = NO;
            self.termContinueSignInView.hidden = YES;
            self.termSignUpContainerView.hidden = NO;
            self.welcomeToTogglLbl.stringValue = @"Welcome to Toggl!";

            // Focus on email when changing mode
            [self.view.window makeFirstResponder:self.email];

            break;

        case TabViewTypeSingup :
            [self.tabView selectTabViewItem:self.loginAndSignUpTabItem];
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
            self.welcomeToTogglLbl.hidden = YES;
            self.welcomeToTogglLbl.stringValue = @"Welcome to Toggl!";
            self.countryStackViewTop.constant = 66;
            self.termContinueSignInView.hidden = YES;
            self.termSignUpContainerView.hidden = NO;
            self.loginWithSSOBtn.hidden = YES;

            // Focus on email when changing mode
            [self.view.window makeFirstResponder:self.email];

            break;

        case TabViewTypeContinueSignin:
            [self.tabView selectTabViewItem:self.loginAndSignUpTabItem];
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

            self.welcomeToTogglLbl.hidden = NO;
            self.subWelcomeLbl.hidden = NO;
            self.socialButtonStackView.hidden = YES;
            self.userActionBtn.title = @"I agree";
            self.email.hidden = YES;
            self.password.hidden = YES;
            self.forgotPasswordTextField.hidden = YES;
            self.leftSeperatorLine.hidden = YES;
            self.rightSeperatorLine.hidden = YES;
            self.orLbl.hidden = YES;
            self.countryStackViewTop.constant = 170;
            self.signUpLink.stringValue = @"Back";
            self.signUpLink.titleUnderline = YES;
            self.termContinueSignInView.hidden = NO;
            self.termSignUpContainerView.hidden = YES;
            self.loginWithSSOBtn.hidden = YES;
            break;
        case TabViewTypeEmailInputSSO:
            [self.tabView selectTabViewItem:self.inputSSOTabItem];
            break;

        case TabViewTypeEmailExistsSSO:
            [self.tabView selectTabViewItem:self.emailExistSSOTabItem];
            break;
    }

    // Change some title if it's SSO authentication
    if (self.isLoginSignUpAsSSO) {
        self.loginWithSSOBtn.hidden = YES;
        self.donotHaveAccountLbl.hidden = YES;
        self.ssoCancelAndGoBackBtn.hidden = NO;
        self.signUpLink.hidden = YES;
        self.ssoTitleLbl.hidden = NO;
        self.appleGoogleGroupViewTop.constant = kSSOLoginAppleViewTop;
    } else {
        self.ssoCancelAndGoBackBtn.hidden = YES;
        self.ssoTitleLbl.hidden = YES;
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
    } else if ([errorStr isEqualToString:@"The operation couldn’t be completed. (com.google.GTMOAuth2 error -1000.)"])
    {
        errorStr = @"Window was closed before login completed.";
    } else if (error.code == -1009) {
        errorStr = @"The Internet connection appears to be offline.";
    } else {
        errorStr = [NSString stringWithFormat:@"Google Authorization Failed. Code %ld", (long)error.code];
    }

    [[NSNotificationCenter defaultCenter] postNotificationOnMainThread:kDisplayError
                                                                object:errorStr];
}

- (void)handleGoogleToken:(NSString *)token
{
    [self showLoaderView:YES];
    self.token = token;

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
            if (![self isValidEmail:self.email.stringValue])
            {
                return NO;
            }
            if (![self isPasswordValid])
            {
                return NO;
            }
            return YES;

        case UserActionAccountSignup :
            if (![self isValidEmail:self.email.stringValue])
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
        case UserActionContinueSSO:
            if (![self isValidEmail:self.emailSSOTextField.stringValue])
            {
                return NO;
            }
            return YES;
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

    if (aNotification.object == self.password && self.currentTab == TabViewTypeSingup) {
        [self validatePasswordStrength];
    }
}

- (BOOL)control:(NSControl *)control textView:(NSTextView *)textView doCommandBySelector:(SEL)commandSelector {
    if (control == self.email || control == self.password || control == self.emailSSOTextField)
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
    self.appleBtn.enabled = !show;
    self.googleBtn.enabled = !show;
}

- (void)resetLoader
{
    [self showLoaderView:NO];
}

- (BOOL)isValidEmail:(NSString *) email
{
    if (email == nil || !email.length || ![email containsString:@"@"])
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
    } else if (self.currentTab == TabViewTypeSingup && ![self.passwordStrengthView isMeetAllRequirements]) {
        [self.password.window makeFirstResponder:self.password];
        [self displayPasswordStrengthView:YES];
        [self validatePasswordStrength];
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
            case TabViewTypeContinueSignin:
                return nil;
            case TabViewTypeEmailInputSSO:
                return nil;
            case TabViewTypeEmailExistsSSO:
                return nil;
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
        case TabViewTypeContinueSignin:
            [self continueBtnOnClick:sender];
            break;
        case TabViewTypeEmailInputSSO:
            [self emailSSOContinueBtnOnClick:sender];
            break;
        case TabViewTypeEmailExistsSSO:
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

    if (self.isLoginSignUpAsSSO)
    {
        AppDelegate *appDelegate = (AppDelegate *)[[NSApplication sharedApplication] delegate];

        if ([email isEqualToString:self.emailSSOTextField.stringValue])
        {
            // user is logging in with same email that he've used for SSO login
            appDelegate.afterLoginMessage = [[SystemMessagePayload alloc] initWithMessage:NSLocalizedString(@"SSO login successfully enabled for your account.",
                                                                                                            @"Show After user is successfully logged in with SSO and existing credentials")
                                                                                  isError:NO];
        } else {
            appDelegate.afterLoginMessage = [[SystemMessagePayload alloc] initWithMessage:NSLocalizedString(@"SSO login for this account was not enabled as login emails were different.",
                                                                                                            @"Show after SSO login, but with existing credentials where email did not match")
                                                                                  isError:YES];
            // this will be default login so we're reseting last SSO operation result
            [[DesktopLibraryBridge shared] resetEnableSSO];
        }
    }
    [[DesktopLibraryBridge shared] loginWithEmail:email password:pass];
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
        case TabViewTypeContinueSignin:
            break;
        case TabViewTypeEmailInputSSO:
            break;
        case TabViewTypeEmailExistsSSO:
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
        case TabViewTypeContinueSignin:
            break;
        case TabViewTypeEmailInputSSO:
            break;
        case TabViewTypeEmailExistsSSO:
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
    self.token = token;
    self.fullName = fullName;

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

- (void)continueBtnOnClick:(id)sender
{
    if (![self validateFormForAction:self.userAction])
    {
        return;
    }
    [self showLoaderView:YES];

    // Continue by Sign Up for new account
    switch (self.userAction) {
        case UserActionAppleLogin:
            toggl_apple_signup_async(ctx, [self.token UTF8String], self.selectedCountryID, [self.fullName UTF8String]);
            break;
        case UserActionGoogleLogin:
            toggl_google_signup_async(ctx, [self.token UTF8String], self.selectedCountryID);
        default:
            break;
    }
}

-(void)continueSignIn
{
    [self changeTabView:TabViewTypeContinueSignin];
    [self.view.window makeFirstResponder:self.countrySelect];
    self.tosCheckbox.state = NSControlStateValueOn;
    [self setUserSignUp:YES];
}

- (void)controlTextDidBeginEditing:(NSNotification *)obj
{
    if (obj.object == self.password && self.currentTab == TabViewTypeSingup) {
        [self displayPasswordStrengthView:YES];
    }
}

- (void)controlTextDidEndEditing:(NSNotification *)obj
{
    if (obj.object == self.password && self.currentTab == TabViewTypeSingup) {
        [self displayPasswordStrengthView:NO];
    }
}

- (void)displayPasswordStrengthView:(BOOL)display {
    if (display) {
        if (self.passwordStrengthView.view.superview == nil) {
            self.passwordStrengthView.view.translatesAutoresizingMaskIntoConstraints = NO;
            [self.view addSubview:self.passwordStrengthView.view];
            [self.passwordStrengthView.view.bottomAnchor constraintEqualToAnchor:self.password.topAnchor constant:kPasswordStrengthViewBottom].active = YES;
            [self.passwordStrengthView.view.centerXAnchor constraintEqualToAnchor:self.view.centerXAnchor constant:kPasswordStrengthViewCenterX].active = YES;
            [self addChildViewController:self.passwordStrengthView];
        }
        self.passwordStrengthView.view.hidden = NO;
    } else {
        self.passwordStrengthView.view.hidden = YES;
    }
}

-(void)setUserAction:(UserAction)userAction {
    _userAction = userAction;
    [self displayPasswordStrengthView:NO];
}

- (void)validatePasswordStrength {
    if (!self.passwordStrengthView.isViewLoaded) {
        return;
    }
    [self.passwordStrengthView updateValidationFor:self.password.stringValue];
}

-(void) loadCountryListIfNeed
{
    if (self.isCountryLoaded) {
        return;
    }

    // Only load if it's sign up view
    switch (self.currentTab) {
        case TabViewTypeSingup:
        case TabViewTypeContinueSignin:
            self.isCountryLoaded = YES;
            toggl_get_countries_async(ctx);
            break;
        default:
            break;
    }
}

#pragma mark - SSO

- (IBAction)emailSSOContinueBtnOnClick:(id)sender
{
    self.userAction = UserActionContinueSSO;

    // Validate all values inserted
    if (![self validateFormForAction:self.userAction])
    {
        return;
    }

    NSString *email = [self.emailSSOTextField stringValue];
    [[DesktopLibraryBridge shared] getSSOIdentityProviderWithEmail:email];
}

- (IBAction)loginToEnableSSOOnClick:(id)sender
{
    [self setShouldEnableSSOAfterLogin:YES];

    // It's the same logic with Login and Sign Up, but different title
    [self changeTabView:TabViewTypeLogin];
}

- (void)linkSSOEmailWithPayload:(SSOPayload *) payload
{
    self.ssoPayload = payload;
    [self changeTabView:TabViewTypeEmailExistsSSO];
    self.email.stringValue = payload.email;
}

- (void)setShouldEnableSSOAfterLogin:(BOOL)enable
{
    self.isLoginSignUpAsSSO = enable;

    if (enable)
    {
        [[DesktopLibraryBridge shared] setNeedEnableSSOWithCode:self.ssoPayload.confirmationCode];
    } else {
        [[DesktopLibraryBridge shared] resetEnableSSO];
        AppDelegate *appDelegate = (AppDelegate *)[[NSApplication sharedApplication] delegate];
        appDelegate.afterLoginMessage = nil;
    }
}

@end
