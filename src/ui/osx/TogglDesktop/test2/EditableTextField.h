//
//  EditableTextField.h
//  TogglDesktop
//
//  Created by Nghia Tran on 12/23/19.
//  Copyright © 2019 Alari. All rights reserved.
//

#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

@interface EditableTextField : NSTextField
@property (assign, nonatomic, readonly) BOOL isTextChanged;
@end

NS_ASSUME_NONNULL_END
