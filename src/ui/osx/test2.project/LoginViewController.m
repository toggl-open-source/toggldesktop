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
#import "Core.h"
#import "GTMOAuth2WindowController.h"

#import "const.h"

@implementation LoginViewController

extern void *ctx;

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

  if (!kopsik_login(ctx, [email UTF8String], [pass UTF8String])) {
    return;
  }
  
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateUserLoggedIn object:nil];
}

// Start Google login.
-(void)textFieldClicked:(id)sender {
  if (sender == self.googleLoginTextField) {
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
    return;
  }

  if (sender == self.passwordForgotTextField) {
    kopsik_password_forgot(ctx);
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

    NSLog(@"Login error: %@", errorStr);

    // User denied access to app
    if ([errorStr isEqualToString:@"access_denied"]) {
      errorStr = @"Access was denied to app";
    }

    [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateError
                                                        object:errorStr];
    return;
  }
  
  if (!kopsik_login(ctx, [auth.accessToken UTF8String], "google_access_token")) {
    return;
  }
  
  [[NSNotificationCenter defaultCenter] postNotificationName:kUIStateUserLoggedIn object:nil];
}

@end
