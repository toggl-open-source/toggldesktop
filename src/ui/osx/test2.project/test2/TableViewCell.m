//
//  TableViewCell.m
//  kopsik_ui_osx
//
//  Created by Tambet Masik on 9/26/13.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import "TableViewCell.h"
#import "UIEvents.h"

@implementation TableViewCell

- (IBAction)continueTimeEntry:(id)sender {
  [[NSNotificationCenter defaultCenter] postNotificationName:kUICommandContinue
                                                      object:self.GUID];
}

- (void)load:(TimeEntryViewItem *)view_item {
  self.GUID = view_item.GUID;
  self.descriptionTextField.stringValue = view_item.Description;
  self.colorTextField.backgroundColor = [self hexCodeToNSColor:view_item.color];
  if (view_item.project) {
    self.projectTextField.stringValue = view_item.project;
    [self.projectTextField setHidden:NO];
  } else {
    self.projectTextField.stringValue = @"";
    [self.projectTextField setHidden:YES];
  }
  self.durationTextField.stringValue = view_item.duration;
}

- (NSColor *)hexCodeToNSColor:(NSString *)hexCode {
	unsigned int colorCode = 0;
  if (hexCode.length > 1) {
    NSString *numbers = [hexCode substringWithRange:NSMakeRange(1, [hexCode length] - 1)];
		NSScanner *scanner = [NSScanner scannerWithString:numbers];
		[scanner scanHexInt:&colorCode];
	}
	return [NSColor
          colorWithDeviceRed:((colorCode>>16)&0xFF)/255.0
          green:((colorCode>>8)&0xFF)/255.0
          blue:((colorCode)&0xFF)/255.0 alpha:1.0];
}

@end
