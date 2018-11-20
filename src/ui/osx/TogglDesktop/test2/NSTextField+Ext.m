//
//  NSTextField+Ext.m
//  TogglDesktop
//
//  Created by Nghia Tran on 11/20/18.
//  Copyright © 2018 Alari. All rights reserved.
//

#import "NSTextField+Ext.h"

@implementation NSTextField(Extension)

-(void)resetCursorColor
{
    NSTextView *textField = (NSTextView *)[self currentEditor];
    if ([textField respondsToSelector:@selector(setInsertionPointColor:)])
    {
        [textField setInsertionPointColor:[self textColor]];
    }
}

@end
