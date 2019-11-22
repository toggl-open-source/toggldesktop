//
//  TouchBar+PrivateAPIs.m
//  TogglDesktop
//
//  Created by Nghia Tran on 11/22/19.
//  Copyright © 2019 Alari. All rights reserved.
//

#import "TouchBar+PrivateAPIs.h"

@interface NSTouchBar ()
/* macOS 10.13 */
+ (void)presentSystemModalFunctionBar:(NSTouchBar *)touchBar
                            placement:(long long)placement
             systemTrayItemIdentifier:(NSTouchBarItemIdentifier)identifier;
+ (void)presentSystemModalFunctionBar:(NSTouchBar *)touchBar
             systemTrayItemIdentifier:(NSTouchBarItemIdentifier)identifier;
+ (void)dismissSystemModalFunctionBar:(NSTouchBar *)touchBar;
+ (void)minimizeSystemModalFunctionBar:(NSTouchBar *)touchBar;

/* macOS 10.14 */
+ (void)presentSystemModalTouchBar:(NSTouchBar *)touchBar
                         placement:(long long)placement
          systemTrayItemIdentifier:(NSTouchBarItemIdentifier)identifier;
+ (void)presentSystemModalTouchBar:(NSTouchBar *)touchBar
          systemTrayItemIdentifier:(NSTouchBarItemIdentifier)identifier;
+ (void)dismissSystemModalTouchBar:(NSTouchBar *)touchBar;
+ (void)minimizeSystemModalTouchBar:(NSTouchBar *)touchBar;
@end

@implementation NSTouchBar (PrivateAPIs)

+ (BOOL)presentSystemModal:(NSTouchBar *)touchBar systemTrayItemIdentifier:(NSTouchBarItemIdentifier)identifier
{
    if ([NSTouchBar respondsToSelector:
         @selector(presentSystemModalFunctionBar:systemTrayItemIdentifier:)])
    {
        [NSTouchBar
         presentSystemModalFunctionBar:touchBar
         systemTrayItemIdentifier:identifier];
        return YES;
    }
    else if ([NSTouchBar respondsToSelector:
              @selector(presentSystemModalTouchBar:systemTrayItemIdentifier:)])
    {
        [NSTouchBar
         presentSystemModalTouchBar:touchBar
         systemTrayItemIdentifier:identifier];
        return YES;
    }
    else
        return NO;
}

+ (void)dismissSystemModal:(NSTouchBar *)touchBar
{
    if ([NSTouchBar respondsToSelector:
         @selector(dismissSystemModalFunctionBar:)])
    {
        [NSTouchBar
         dismissSystemModalFunctionBar:touchBar];
    }
    else if ([NSTouchBar respondsToSelector:
              @selector(dismissSystemModalTouchBar:)])
    {
        [NSTouchBar
         dismissSystemModalTouchBar:touchBar];
    }
}

+ (void)minimizeSystemModal:(NSTouchBar *)touchBar
{
    if ([NSTouchBar respondsToSelector:
         @selector(minimizeSystemModalFunctionBar:)])
    {
        [NSTouchBar
         minimizeSystemModalFunctionBar:touchBar];
    }
    else if ([NSTouchBar respondsToSelector:
              @selector(minimizeSystemModalTouchBar:)])
    {
        [NSTouchBar
         minimizeSystemModalTouchBar:touchBar];
    }
}
@end
