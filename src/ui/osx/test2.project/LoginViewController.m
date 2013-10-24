//
//  LoginViewController.m
//  kopsik_ui_osx
//
//  Created by Tambet Masik on 9/24/13.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import "LoginViewController.h"
#import "kopsik_api.h"
#import "UIEvents.h"
#import "Context.h"
#import "GTMOAuth2WindowController.h"

@interface LoginViewController ()

@end

@implementation LoginViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
    }
    return self;
}

-(void)awakeFromNib {
  // Set google login link color to blue
  NSColor *color = [NSColor blueColor];
  NSMutableAttributedString *colorTitle =
    [[NSMutableAttributedString alloc] initWithAttributedString:[self.loginWithGoogleButton attributedTitle]];
  NSRange titleRange = NSMakeRange(0, [colorTitle length]);
  [colorTitle addAttribute:NSForegroundColorAttributeName value:color range:titleRange];
  [self.loginWithGoogleButton setAttributedTitle:colorTitle];
}

- (IBAction)clickLoginButton:(id)sender {
  NSString *email = [self.email stringValue];
  NSString *pass = [self.password stringValue];
  [self.password setStringValue:@""];
  char err[KOPSIK_ERR_LEN];
  if (KOPSIK_API_SUCCESS != kopsik_login(ctx, err, KOPSIK_ERR_LEN, [email UTF8String], [pass UTF8String])) {
    NSLog(@"Login error: %s", err);
    [self.errorLabel setStringValue:[NSString stringWithUTF8String:err]];
    [self.errorLabel setHidden:NO];
    return;
  }
  
  [self.errorLabel setHidden:YES];
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateUserLoggedIn object:nil];
}

- (IBAction)clickGoogleLoginButton:(id)sender {
  
  NSString *scope = @"https://www.googleapis.com/auth/userinfo.profile https://www.googleapis.com/auth/userinfo.email";
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
}

- (IBAction)clickProxySettingsButton:(id)sender {
  [[NSNotificationCenter defaultCenter] postNotificationName:kUICommandShowPreferences object:nil];
}

- (void)viewController:(GTMOAuth2WindowController *)viewController
      finishedWithAuth:(GTMOAuth2Authentication *)auth
                 error:(NSError *)error {
  if (error != nil) {
    NSString *errorStr = [error localizedDescription];
    
    NSData *responseData = [[error userInfo] objectForKey:kGTMHTTPFetcherStatusDataKey];
    if ([responseData length] > 0) {
      // Show the body of the server's authentication failure response
      errorStr = [[NSString alloc] initWithData:responseData
                                        encoding:NSUTF8StringEncoding];
    } else {
      NSString *str = [[error userInfo] objectForKey:kGTMOAuth2ErrorMessageKey];
      if (str != nil) {
        if ([str length] > 0) {
          errorStr = str;
        }
      }
    }
    if ([errorStr isEqualToString:@"access_denied"]) {
      // User canceled login, nothing to do here.
      return;
    }
    NSLog(@"Login error: %@", errorStr);
    [self.errorLabel setStringValue:errorStr];
    [self.errorLabel setHidden:NO];
    return;
  }
  
  char err[KOPSIK_ERR_LEN];
  if (KOPSIK_API_SUCCESS != kopsik_login(ctx, err, KOPSIK_ERR_LEN, [auth.accessToken UTF8String], "google_access_token")) {
    NSLog(@"Login error: %s", err);
    [self.errorLabel setStringValue:[NSString stringWithUTF8String:err]];
    [self.errorLabel setHidden:NO];
    return;
  }
  
  [self.errorLabel setHidden:YES];
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateUserLoggedIn object:nil];
}

@end