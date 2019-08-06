//
//  AutoCompleteTable.h
//  LiteComplete
//
//  Created by Indrek Vändrik on 21/02/2018.
//  Copyright © 2018 Toggl. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class AutoCompleteTableCell;

@interface AutoCompleteTable : NSTableView
@property (nonatomic, assign, readonly) NSInteger lastSelected;
@property (nonatomic, assign, readonly) NSInteger lastClicked;
@property (nonatomic, assign) NSInteger lastSavedSelected;

- (void)nextItem;
- (void)previousItem;
- (AutoCompleteTableCell *)getSelectedCell:(NSInteger)row;
- (void)setFirstRowAsSelected;
- (void)setCurrentSelected:(NSInteger)index next:(BOOL)isNext;
- (void)resetSelected;
@end
