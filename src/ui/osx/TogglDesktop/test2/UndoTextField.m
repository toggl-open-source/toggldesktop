//
//  UndoTextField.m
//  TogglDesktop
//
//  Created by Nghia Tran on 12/13/18.
//  Copyright © 2018 Alari. All rights reserved.
//

#import "UndoTextField.h"

@interface UndoTextField ()
@property (strong, nonatomic) NSUndoManager *undo;
@end

@implementation UndoTextField

- (NSUndoManager *)undoManager {
	if (self.undo == nil)
	{
		self.undo = [[NSUndoManager alloc] init];
	}
	return self.undo;
}

- (void)registerUndoWithValue:(NSString *_Nullable)value
{
	if (value == nil)
	{
		return;
	}

	[self.undoManager removeAllActionsWithTarget:self];
	[self.undoManager registerUndoWithTarget:self
									selector:@selector(setStringInput:)
									  object:value];
}

- (void)setStringInput:(NSString *)value
{
	NSString *oldValue = [self.stringValue copy];

	self.stringValue = value;
	[self.undoManager registerUndoWithTarget:self
									selector:@selector(setStringInput:)
									  object:oldValue];
}

@end
