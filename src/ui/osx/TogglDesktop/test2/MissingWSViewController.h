//
//  MissingWSViewController.h
//  TogglDesktop
//
//  Created by Indrek Vändrik on 31/08/2017.
//  Copyright © 2017 Alari. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "NSTextFieldClickablePointer.h"

@interface MissingWSViewController : NSViewController
@property (weak) IBOutlet NSButton *loginButton;
@property (weak) IBOutlet NSTextFieldClickablePointer *syncLink;
@end
