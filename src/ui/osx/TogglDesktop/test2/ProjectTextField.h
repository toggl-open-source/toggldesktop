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
@class AutocompleteItem;

@interface ProjectTextField : NSTextField

@property (assign, nonatomic) BOOL isInTimerBar;
@property (assign, nonatomic) BOOL renderClient;
@property (assign, nonatomic) BOOL renderTask;

- (void)setTitleWithTimeEntry:(TimeEntryViewItem *)item;
- (void)setTitleWithAutoCompleteItem:(AutocompleteItem *)item;

@end

NS_ASSUME_NONNULL_END
