//
//  AutocompleteItem.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 18/11/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import "AutocompleteItem.h"

@implementation AutocompleteItem

- (void)load:(KopsikAutocompleteItem *)data {
  self.Text = [NSString stringWithUTF8String:data->Text];
  self.TimeEntryID = data->TimeEntryID;
  self.ProjectID = data->ProjectID;
  self.TaskID = data->TaskID;
}

- (void)save:(KopsikAutocompleteItem *)data {
  NSAssert(!data->Text, @"data already has text");
  data->Text = strdup([self.Text UTF8String]);
  data->TimeEntryID = (unsigned int)self.TimeEntryID;
  data->ProjectID = (unsigned int)self.ProjectID;
  data->TaskID = (unsigned int)self.TaskID;
}

- (NSString *)description {
  return [NSString stringWithFormat:@"Text: %@, PID: %ld, TID: %ld, TE ID: %ld",
          self.Text, self.ProjectID, self.TaskID, self.TimeEntryID];
}

@end
