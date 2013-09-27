//
//  TimerEditViewController.h
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface TimerEditViewController : NSViewController
@property (weak) IBOutlet NSTextField *descriptionTextField;
@property (weak) IBOutlet NSButton *startButton;

@end
