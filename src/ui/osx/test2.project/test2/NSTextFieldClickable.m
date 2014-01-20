//
//  NSTextFieldClickable.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 11/11/2013.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import "NSTextFieldClickable.h"

@implementation NSTextFieldClickable

- (void)mouseDown:(NSEvent *)theEvent {
  [self sendAction:@selector(textFieldClicked:) to:[self delegate]];
}

@end
