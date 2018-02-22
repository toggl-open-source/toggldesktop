//
//  AutoCompleteTable.h
//  LiteComplete
//
//  Created by Indrek Vändrik on 21/02/2018.
//  Copyright © 2018 Toggl. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface AutoCompleteTable : NSTableView
@property BOOL listVisible;
- (void)setFirstRowAsSelected;
@end
