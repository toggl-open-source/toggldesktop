//
//  ClickableImageView.h
//  TogglDesktop
//
//  Created by Nghia Tran on 9/16/19.
//  Copyright © 2019 Alari. All rights reserved.
//

#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

@protocol ClickableImageViewDelegate <NSObject>

- (void)imageViewOnClick:(id)sender;

@end

@interface ClickableImageView : NSImageView

@property (weak, nonatomic) id<ClickableImageViewDelegate> delegate;

@end

NS_ASSUME_NONNULL_END
