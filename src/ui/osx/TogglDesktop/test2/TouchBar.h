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

@interface NSTouchBar ()

+ (void)presentSystemModalFunctionBar:(NSTouchBar *)touchBar systemTrayItemIdentifier:(NSString *)identifier;

@end
#endif

#endif /* TouchBar_h */
