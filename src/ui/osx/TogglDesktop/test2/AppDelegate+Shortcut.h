//
//  AppDelegate+Shortcut.h
//  TogglDesktop
//
//  Created by Nghia Tran on 11/20/18.
//  Copyright © 2018 Alari. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "AppDelegate.h"

@class Settings;

NS_ASSUME_NONNULL_BEGIN

@interface AppDelegate(Shortcut)

/**
 Handle all shortcut of the app depend on specific Setting

 @param setting Settings model
 */
-(void) handleShortcutForSetting:(Settings *) setting;

@end

NS_ASSUME_NONNULL_END
