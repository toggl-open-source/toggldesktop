//
//  TouchBar.h
//  TogglDesktop
//
//  Created by Indrek Vändrik on 24/02/2019.
//  Copyright © 2019 Alari. All rights reserved.
//

#ifndef TouchBar_h
#define TouchBar_h

#ifndef APP_STORE
#import <AppKit/AppKit.h>

extern void DFRElementSetControlStripPresenceForIdentifier(NSString *, BOOL) __OSX_AVAILABLE_STARTING(__MAC_10_12_2,__IPHONE_NA);
extern void DFRSystemModalShowsCloseBoxWhenFrontMost(BOOL) __OSX_AVAILABLE_STARTING(__MAC_10_12_2,__IPHONE_NA);

@interface NSTouchBarItem ()

+ (void)addSystemTrayItem:(NSTouchBarItem *)item __OSX_AVAILABLE_STARTING(__MAC_10_12_2,__IPHONE_NA);
+ (void)removeSystemTrayItem:(NSTouchBarItem *)item __OSX_AVAILABLE_STARTING(__MAC_10_12_2,__IPHONE_NA);

@end

@interface NSTouchBar (PrivateAPIs)

+ (BOOL)presentSystemModal:(NSTouchBar *)touchBar systemTrayItemIdentifier:(NSTouchBarItemIdentifier)identifier __OSX_AVAILABLE_STARTING(__MAC_10_12_2,__IPHONE_NA);

+ (void)dismissSystemModal:(NSTouchBar *)touchBar __OSX_AVAILABLE_STARTING(__MAC_10_12_2,__IPHONE_NA);

+ (void)minimizeSystemModal:(NSTouchBar *)touchBar __OSX_AVAILABLE_STARTING(__MAC_10_12_2,__IPHONE_NA);

@end

#endif
#endif /* TouchBar_h */

