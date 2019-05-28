//
//  UndoTextField.h
//  TogglDesktop
//
//  Created by Nghia Tran on 12/13/18.
//  Copyright © 2018 Alari. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface UndoTextField : NSTextField
- (void)registerUndoWithValue:(NSString *_Nullable)value;
@end
