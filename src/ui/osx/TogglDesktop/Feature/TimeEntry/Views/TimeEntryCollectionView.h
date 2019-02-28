//
//  TimeEntryCollectionView.h
//  TogglDesktop
//
//  Created by Nghia Tran on 2/20/19.
//  Copyright © 2019 Alari. All rights reserved.
//

#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

@interface TimeEntryCollectionView : NSCollectionView

@property (strong, nonatomic) NSIndexPath *clickedIndexPath;

@end

NS_ASSUME_NONNULL_END
