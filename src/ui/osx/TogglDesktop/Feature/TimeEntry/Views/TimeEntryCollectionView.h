//
//  TimeEntryCollectionView.h
//  TogglDesktop
//
//  Created by Nghia Tran on 2/20/19.
//  Copyright © 2019 Alari. All rights reserved.
//

#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

@class TimeEntryCell;

@interface TimeEntryCollectionView : NSCollectionView

@property (assign, nonatomic) BOOL isUserAction;
@property (strong, nonatomic) NSIndexPath *clickedIndexPath;

- (TimeEntryCell *)getSelectedEntryCell;

@end

NS_ASSUME_NONNULL_END
