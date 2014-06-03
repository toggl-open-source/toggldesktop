//
//  Update.m
//  kopsik_ui_osx
//
//  Created by Tanel Lebedev on 29/10/2013.
//  Copyright (c) 2013 TogglDesktop developers. All rights reserved.
//

#import "Update.h"

@implementation Update

- (void)load:(KopsikUpdateViewItem *)view
{
	self.channel = [NSString stringWithUTF8String:view->update_channel];
	self.is_checking = view->is_checking;
	self.is_update_available = view->is_update_available;
	self.URL = [NSString stringWithUTF8String:view->url];
	self.version = [NSString stringWithUTF8String:view->version];
}

@end
