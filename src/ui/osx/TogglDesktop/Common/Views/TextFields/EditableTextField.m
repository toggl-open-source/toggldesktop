//
//  EditableTextField.m
//  TogglDesktop
//
//  Created by Nghia Tran on 12/23/19.
//  Copyright © 2019 Alari. All rights reserved.
//

#import "EditableTextField.h"

@interface EditableTextField ()
@property (assign, nonatomic) BOOL isTextChanged;
@end

@implementation EditableTextField

- (void)textDidChange:(NSNotification *)notification
{
    [super textDidChange:notification];
    self.isTextChanged = YES;
}

- (void)textDidEndEditing:(NSNotification *)notification
{
    [super textDidEndEditing:notification];
    self.isTextChanged = NO;
}

@end
