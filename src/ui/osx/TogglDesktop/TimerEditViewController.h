//
//  TimerEditViewController.h
//  Toggl Desktop on the Mac
//
//  Created by Tanel Lebedev on 19/09/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class BetterFocusAutoCompleteInput;
@class AutocompleteItem;

@interface TimerEditViewController : NSViewController <NSComboBoxDataSource, NSTextFieldDelegate, NSTableViewDelegate>
@property (weak) IBOutlet BetterFocusAutoCompleteInput *autoCompleteInput;
@property (strong, nonatomic) NSArray *projectComboConstraint;
@property (strong, nonatomic) NSArray *projectLabelConstraint;

- (void)timerFired:(NSTimer *)timer;
- (void)fillEntryFromAutoComplete:(AutocompleteItem *)item;

@end
