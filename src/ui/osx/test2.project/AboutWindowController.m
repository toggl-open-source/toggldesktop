//
//  AboutWindowController.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 29/10/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import "AboutWindowController.h"

@interface AboutWindowController ()

@end

@implementation AboutWindowController

- (id)initWithWindow:(NSWindow *)window
{
    self = [super initWithWindow:window];
    if (self) {
        // Initialization code here.
    }
    return self;
}

- (void)windowDidLoad
{
  [super windowDidLoad];
    
  NSDictionary* infoDict = [[NSBundle mainBundle] infoDictionary];
  NSString* version = [infoDict objectForKey:@"CFBundleShortVersionString"];
  [self.versionTextField setStringValue:[NSString stringWithFormat:@"Version %@", version]];
  NSString* appname = [infoDict objectForKey:@"CFBundleName"];
  [self.appnameTextField setStringValue:appname];

  NSString *path = [[NSBundle mainBundle] pathForResource:@"Credits" ofType:@"rtf"];
  [self.creditsTextView readRTFDFromFile:path];
  
}

- (IBAction)checkForUpdateClicked:(id)sender {
}
@end
