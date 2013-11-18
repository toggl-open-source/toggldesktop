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
}

- (NSString *)description {
  return [NSString stringWithFormat:@"Text: %@, CID: %ld, PID: %ld, TID: %ld, TE ID: %ld",
          self.Text, self.ClientID, self.ProjectID, self.TaskID, self.TimeEntryID];
}


@end
