//
//  LoginViewController.m
//  kopsik_ui_osx
//
//  Created by Tambet Masik on 9/24/13.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "LoginViewController.h"
#import "kopsik_api.h"
#import "UIEvents.h"
#import "Context.h"
#import "GTMOAuth2WindowController.h"

#import "const.h"

@interface LoginViewController ()

@end

@implementation LoginViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
      [self performSelector:@selector(setDefaultUser) withObject:nil afterDelay:0.5];
    }
    return self;
}

- (void)setDefaultUser {
  if (defaultEmail != nil) {
    [self.email setStringValue:defaultEmail];
  }
  if (defaultPassword != nil) {
    [self.password setStringValue:defaultPassword];
  }
}

- (IBAction)clickLoginButton:(id)sender {
  NSString *email = [self.email stringValue];
  if (email == nil || !email.length) {
    [self.email becomeFirstResponder];
    return;
  }

  NSString *pass = [self.password stringValue];
  if (pass == nil || !pass.length) {
    [self.password becomeFirstResponder];
    return;
  }

  [self.password setStringValue:@""];

  char err[KOPSIK_ERR_LEN];
  if (KOPSIK_API_SUCCESS != kopsik_login(ctx, err, KOPSIK_ERR_LEN, [email UTF8String], [pass UTF8String])) {
    NSLog(@"Login error: %s", err);
    NSString *msg = [NSString stringWithUTF8String:err];
    if ([msg rangeOfString:@"Request to server failed with status code: 403"].location != NSNotFound) {
      msg = @"Invalid e-mail or password. Please try again!";
    }
    [self.errorLabel setStringValue:msg];
    [self.troubleBox setHidden:NO];
    return;
  }
  
  [self.troubleBox setHidden:YES];
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateUserLoggedIn object:nil];
}

// Start Google login.
-(void)textFieldClicked:(id)sender {
  if (sender == self.googleLoginTextField) {
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
    return;
  }

  if (sender == self.passwordForgotTextField) {
    NSString *lostPasswordURL = [NSString stringWithUTF8String:kLostPasswordURL];
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:lostPasswordURL]];
    return;
  }
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
    [self.troubleBox setHidden:NO];
    return;
  }
  
  char err[KOPSIK_ERR_LEN];
  if (KOPSIK_API_SUCCESS != kopsik_login(ctx, err, KOPSIK_ERR_LEN, [auth.accessToken UTF8String], "google_access_token")) {
    NSLog(@"Login error: %s", err);
    [self.errorLabel setStringValue:[NSString stringWithUTF8String:err]];
    [self.troubleBox setHidden:NO];
    return;
  }
  
  [self.troubleBox setHidden:YES];
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateUserLoggedIn object:nil];
}

@end
