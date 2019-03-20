//
//  ProjectTextField.h
//  TogglDesktop
//
//  Created by Nghia Tran on 3/5/19.
//  Copyright © 2019 Alari. All rights reserved.
//

#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

@class TimeEntryViewItem;

@interface ProjectTextField : NSTextField

@property (assign, nonatomic) BOOL isInTimerBar;

- (void)setTitleWithTimeEntry:(TimeEntryViewItem *)item;

@end

NS_ASSUME_NONNULL_END
