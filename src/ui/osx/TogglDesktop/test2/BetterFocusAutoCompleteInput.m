//
//  BetterFocusAutoCompleteInput.m
//  TogglDesktop
//
//  Created by Nghia Tran on 3/7/19.
//  Copyright © 2019 Alari. All rights reserved.
//

#import "BetterFocusAutoCompleteInput.h"
#import "FocusCell.h"

@implementation BetterFocusAutoCompleteInput

+ (Class)cellClass
{
	return [FocusCell class];
}

- (NSRect)focusRingMaskBounds
{
	return [FocusCell frameFocusRingFromFrame:self.bounds];
}

@end
