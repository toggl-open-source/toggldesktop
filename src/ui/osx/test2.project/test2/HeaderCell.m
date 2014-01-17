//
//  TableGroupCell.m
//  kopsik_ui_osx
//
//  Created by Tambet Masik on 10/24/13.
//  Copyright (c) 2013 kopsik developers. All rights reserved.
//

#import "HeaderCell.h"

@implementation HeaderCell

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code here.
    }
    return self;
}

- (void)drawRect:(NSRect)dirtyRect {
	[super drawRect:dirtyRect];
	
    // Drawing code here.
}

- (void)load:(DateHeader *)header {
  [self.nameTextField setStringValue:[header description]];
}

@end
