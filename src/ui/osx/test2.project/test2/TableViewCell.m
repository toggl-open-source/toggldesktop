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
  if (view_item.project) {
    self.projectTextField.stringValue = [view_item.project uppercaseString];
    [self.projectTextField setHidden:NO];
  } else {
    self.projectTextField.stringValue = @"";
    [self.projectTextField setHidden:YES];
  }
  self.durationTextField.stringValue = view_item.duration;
}

@end
