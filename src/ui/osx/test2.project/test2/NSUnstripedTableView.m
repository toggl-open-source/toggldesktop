//
//  NSUnstripedTableView.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 04/02/2014.
//  Copyright (c) 2014 kopsik developers. All rights reserved.
//

#import "NSUnstripedTableView.h"

@implementation NSUnstripedTableView

- (void)drawGridInClipRect:(NSRect)clipRect {
  NSRect lastRowRect = [self rectOfRow:[self numberOfRows]-1];
  NSRect myClipRect = NSMakeRect(0, 0, lastRowRect.size.width, NSMaxY(lastRowRect));
  NSRect finalClipRect = NSIntersectionRect(clipRect, myClipRect);
  [super drawGridInClipRect:finalClipRect];
}

@end
