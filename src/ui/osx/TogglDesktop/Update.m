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
	self.channel = [NSString stringWithUTF8String:view->UpdateChannel];
	self.is_checking = view->IsChecking;
	self.is_update_available = view->IsUpdateAvailable;
	self.URL = [NSString stringWithUTF8String:view->URL];
	self.version = [NSString stringWithUTF8String:view->Version];
}

@end
