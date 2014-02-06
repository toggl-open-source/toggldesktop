//
//  TableViewCell.m
//  kopsik_ui_osx
//
//  Created by Tambet Masik on 9/26/13.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import "TimeEntryCell.h"
#import "UIEvents.h"
#import "ConvertHexColor.h"

@implementation TimeEntryCell

- (IBAction)continueTimeEntry:(id)sender {
  [[NSNotificationCenter defaultCenter] postNotificationName:kUICommandContinue
                                                      object:self.GUID];
}

- (void)render:(TimeEntryViewItem *)view_item {
  NSAssert([NSThread isMainThread], @"Rendering stuff should happen on main thread");

  self.GUID = view_item.GUID;
  self.descriptionTextField.stringValue = view_item.Description;
  self.durationTextField.stringValue = view_item.duration;
  
  // Time entry has a project
  if (view_item.ProjectAndTaskLabel && [view_item.ProjectAndTaskLabel length] > 0) {
    self.projectTextField.stringValue = [view_item.ProjectAndTaskLabel uppercaseString];
    [self.projectTextField setHidden:NO];
    self.projectTextField.backgroundColor =
      [ConvertHexColor hexCodeToNSColor:view_item.ProjectColor];
    return;
  }

  // Time entry has no project
  self.projectTextField.stringValue = @"";
  [self.projectTextField setHidden:YES];
}

@end
