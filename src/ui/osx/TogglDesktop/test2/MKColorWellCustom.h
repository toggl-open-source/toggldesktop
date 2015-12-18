//
//  MKColorWell+Custom.m
//  TogglDesktop
//
//  Created by Indrek Vändrik on 17/12/15.
//  Copyright (c) 2015 Toggl Desktop developers. All rights reserved.
//

#import "MKColorWell.h"

@interface MKColorWellCustom : MKColorWell
@property NSMutableArray *colors;
- (void)setProjectColors:(NSMutableArray *)colors;
@end
