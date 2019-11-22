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

extern void DFRElementSetControlStripPresenceForIdentifier(NSString *, BOOL);
extern void DFRSystemModalShowsCloseBoxWhenFrontMost(BOOL);

@interface NSTouchBarItem ()

+ (void)addSystemTrayItem:(NSTouchBarItem *)item;
+ (void)removeSystemTrayItem:(NSTouchBarItem *)item;

@end

@interface NSTouchBar (PrivateAPIs)

+ (BOOL)presentSystemModal:(NSTouchBar *)touchBar
                 placement:(long long)placement
  systemTrayItemIdentifier:(NSTouchBarItemIdentifier)identifier;

+ (void)dismissSystemModal:(NSTouchBar *)touchBar;

#endif

@end

#endif /* TouchBar_h */
