//
//  UndoTextField.h
//  TogglDesktop
//
//  Created by Nghia Tran on 12/13/18.
//  Copyright © 2018 Alari. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface UndoTextField : NSTextField
- (void)registerUndoWithValue:(NSString *)value;
@end

NS_ASSUME_NONNULL_END
