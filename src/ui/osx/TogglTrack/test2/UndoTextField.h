//
//  UndoTextField.h
//  TogglTrack
//
//  Created by Nghia Tran on 12/13/18.
//  Copyright © 2018 Alari. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "EditableTextField.h"

@interface UndoTextField : EditableTextField
- (void)registerUndoWithValue:(NSString *_Nullable)value;
@end
