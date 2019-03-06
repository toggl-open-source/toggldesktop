//
//  ProjectTextField.h
//  TogglDesktop
//
//  Created by Nghia Tran on 3/5/19.
//  Copyright © 2019 Alari. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "NSTextFieldClickable.h"

NS_ASSUME_NONNULL_BEGIN

@class TimeEntryViewItem;

@interface ProjectTextField : NSTextFieldClickable

- (void)setTitleWithTimeEntry:(TimeEntryViewItem *)item;

@end

NS_ASSUME_NONNULL_END
