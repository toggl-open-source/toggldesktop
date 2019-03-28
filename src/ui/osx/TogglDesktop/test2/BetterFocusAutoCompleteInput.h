//
//  BetterFocusAutoCompleteInput.h
//  TogglDesktop
//
//  Created by Nghia Tran on 3/7/19.
//  Copyright © 2019 Alari. All rights reserved.
//

#import "AutoCompleteInput.h"
#import "TogglDesktop-Swift.h"

NS_ASSUME_NONNULL_BEGIN

@interface BetterFocusAutoCompleteInput : AutoCompleteInput
@property (weak, nonatomic) id<TextFieldResponderDelegate> responderDelegate;
@end

NS_ASSUME_NONNULL_END
