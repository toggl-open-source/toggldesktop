#import "MASShortcut.h"

@interface MASShortcut (Monitoring)

+ (id)addGlobalHotkeyMonitorWithShortcut:(MASShortcut *)shortcut handler:(void (^)())handler;
+ (void)removeGlobalHotkeyMonitor:(id)monitor;

@end
