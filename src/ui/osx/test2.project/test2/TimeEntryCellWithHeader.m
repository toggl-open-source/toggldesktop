//
//  TableViewCell.m
//  kopsik_ui_osx
//
//  Created by Tambet Masik on 9/26/13.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import "TimeEntryCellWithHeader.h"
#import "UIEvents.h"
#import "ConvertHexColor.h"

@implementation TimeEntryCellWithHeader

- (IBAction)continueTimeEntry:(id)sender {
  [[NSNotificationCenter defaultCenter] postNotificationName:kUICommandContinue
                                                      object:self.GUID];
}

- (void)load:(TimeEntryViewItem *)view_item {
  self.GUID = view_item.GUID;
  self.descriptionTextField.stringValue = view_item.Description;
  self.durationTextField.stringValue = view_item.duration;
  self.dateDurationTextField.stringValue = view_item.dateDuration;
  self.formattedDateTextField.stringValue = view_item.formattedDate;
  
  // Time entry has a project
  if (view_item.ProjectAndTaskLabel && [view_item.ProjectAndTaskLabel length] > 0) {
    self.projectTextField.stringValue = [view_item.ProjectAndTaskLabel uppercaseString];
    [self.projectTextField setHidden:NO];
    self.projectTextField.backgroundColor = [ConvertHexColor hexCodeToNSColor:view_item.color];
    return;
  }

  // Time entry has no project
  self.projectTextField.stringValue = @"";
  [self.projectTextField setHidden:YES];
}

@end
