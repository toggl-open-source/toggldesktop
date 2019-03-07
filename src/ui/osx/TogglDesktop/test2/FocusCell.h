//
//  FocusCell.h
//  TogglDesktop
//
//  Created by Nghia Tran on 3/7/19.
//  Copyright © 2019 Alari. All rights reserved.
//

#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

@interface FocusCell : NSTextFieldCell

+ (CGRect)frameFocusRingFromFrame:(CGRect)frame;

@end

NS_ASSUME_NONNULL_END
