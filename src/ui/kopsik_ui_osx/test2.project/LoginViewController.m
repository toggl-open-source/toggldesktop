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
        // Initialization code here.
    }
    return self;
}

- (IBAction)clickLoginButton:(id)sender {
  NSString *email = [self.email stringValue];
  NSString *pass = [self.password stringValue];
  char err[KOPSIK_ERR_LEN];
  if (KOPSIK_API_SUCCESS != kopsik_login(ctx, err, KOPSIK_ERR_LEN, [email UTF8String], [pass UTF8String])) {
    [self.errorLabel setStringValue:[NSString stringWithUTF8String:err]];
    [self.errorLabel setHidden:NO];
  } else {
    [self.errorLabel setHidden:YES];
    [[NSNotificationCenter defaultCenter] postNotificationName:kUIEventUserLoggedIn object:nil];
  }
}

- (IBAction)clickGoogleLoginButton:(id)sender {
  
  NSString *scope = @"https://www.googleapis.com/auth/userinfo.email";
  NSString *clientID = @"426090949585-uj7lka2mtanjgd7j9i6c4ik091rcv6n5.apps.googleusercontent.com";
  // According to Google docs, in installed apps the client secret is not expected to stay secret:
  NSString *clientSecret = @"TsQlSHqGEYSgoSZe91E2pMtp";
  
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

- (void)viewController:(GTMOAuth2WindowController *)viewController
      finishedWithAuth:(GTMOAuth2Authentication *)auth
                 error:(NSError *)error {
  if (error != nil) {
    // Authentication failed
  } else {
    // Authentication succeeded
  }
}

@end