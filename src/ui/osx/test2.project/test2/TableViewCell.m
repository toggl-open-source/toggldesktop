//
//  TableViewCell.m
//  kopsik_ui_osx
//
//  Created by Tambet Masik on 9/26/13.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import "TableViewCell.h"
#import "UIEvents.h"
#import "ConvertHexColor.h"

@implementation TableViewCell

- (IBAction)continueTimeEntry:(id)sender {
  [[NSNotificationCenter defaultCenter] postNotificationName:kUICommandContinue
                                                      object:self.GUID];
}

- (void)load:(TimeEntryViewItem *)view_item {
  self.GUID = view_item.GUID;
  self.descriptionTextField.stringValue = view_item.Description;
  self.colorTextField.backgroundColor = [ConvertHexColor hexCodeToNSColor:view_item.color];
  self.durationTextField.stringValue = view_item.duration;
  
  // Time entry has a project
  if (view_item.ProjectAndTaskLabel && [view_item.ProjectAndTaskLabel length] > 0) {
    self.projectTextField.stringValue = [view_item.ProjectAndTaskLabel uppercaseString];
    [self.projectTextField setHidden:NO];
    return;
  }

  // Time entry has no project
  self.projectTextField.stringValue = @"";
  [self.projectTextField setHidden:YES];
}

@end
