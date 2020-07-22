//
//  LoginViewController.h
//  Toggl Desktop on the Mac
//
//  Created by Tambet Masik on 9/24/13.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class SSOPayload;

@interface LoginViewController : NSViewController
- (void)resetLoader;
- (void)continueSignIn;
- (void)linkSSOEmailWithPayload:(SSOPayload *) payload;
@end
