//
//  MainWindowController.h
//  kopsik_ui_osx
//
//  Created by Tambet Masik on 9/24/13.
//  Copyright (c) 2013 Alari. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface MainWindowController : NSWindowController
@property (weak) IBOutlet NSView *headerView;
@property (weak) IBOutlet NSView *contentView;
@property (weak) IBOutlet NSView *footerView;

@end
