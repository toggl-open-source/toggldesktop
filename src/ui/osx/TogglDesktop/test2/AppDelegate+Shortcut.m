//
//  AppDelegate+Shortcut.m
//  TogglDesktop
//
//  Created by Nghia Tran on 11/20/18.
//  Copyright © 2018 Alari. All rights reserved.
//

#import "AppDelegate+Shortcut.h"
#import "Settings.h"

@implementation AppDelegate(Shortcut)

-(void) handleShortcutForSetting:(Settings *) setting
{
    // Enable Command+Q if the Dock is visible
    // Otherwise, disable it if Dock is invisible -> We have to quit explicitly in Status Menu
    self.quitMenuItem.keyEquivalent = setting.dock_icon ?  @"q" : @"";
}
@end
