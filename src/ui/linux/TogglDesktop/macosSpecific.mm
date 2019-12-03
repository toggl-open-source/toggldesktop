#include <QApplication>
#include <QWidget>

#include <AppKit/AppKit.h>

void changeTitleBarColor(WId id, double r, double g, double b) {
    NSView *view = (NSView*) id;
    NSWindow* window = [view window];
    window.titlebarAppearsTransparent = YES;
    window.backgroundColor = [NSColor colorWithRed:r green:g blue:b alpha:1.];
}
