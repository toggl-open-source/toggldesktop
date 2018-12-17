#include "platforminfo.h"

#if defined(__linux)

#include <cstdio>

#include <X11/Xlib.h>
#include <X11/Xmu/WinUtil.h>
#include <X11/Xatom.h>

#define LONG_LENGTH 512

void getWhatever() {
    Display *display { nullptr };
    Window *window { nullptr };
    Atom propertyName { 0 };
    Atom propertyType { 0 };
    Atom returnType { 0 };
    int returnFormat { 0 };
    unsigned long returnCount { 0 };
    unsigned long returnOffset { 0 };
    char *wmName { nullptr };

    display = XOpenDisplay(nullptr);
    if (display) {
        // get WM window attempt number 1
        propertyName = XInternAtom(display, "_NET_SUPPORTING_WM_CHECK", False);
        propertyType = XA_WINDOW;
        if (XGetWindowProperty(display,
                               DefaultRootWindow(display),
                               propertyName,
                               0,
                               LONG_LENGTH,
                               False,
                               propertyType,
                               &returnType,
                               &returnFormat,
                               &returnCount,
                               &returnOffset,
                               reinterpret_cast<unsigned char**>(&window)
                              ) == Success) {
            if (returnType != propertyType) {
                XFree(window);
                window = nullptr;
            }
        }

        // get WM window attempt number 2
        if (!window) {
            propertyName = XInternAtom(display, "_WIN_SUPPORTING_WM_CHECK", False);
            propertyType = XA_CARDINAL;
            if (XGetWindowProperty(display,
                                   DefaultRootWindow(display),
                                   propertyName,
                                   0,
                                   LONG_LENGTH,
                                   False,
                                   propertyType,
                                   &returnType,
                                   &returnFormat,
                                   &returnCount,
                                   &returnOffset,
                                   reinterpret_cast<unsigned char**>(&window)
                                  ) == Success) {
                if (returnType != propertyType) {
                    XFree(window);
                    window = nullptr;
                }
            }
        }

        // if we managed to get the WM window:
        if (window) {
            // attempt number 1 to get its name (UTF version)
            propertyName = XInternAtom(display, "_NET_WM_NAME", False);
            propertyType = XInternAtom(display, "UTF8_STRING", False);
            if (XGetWindowProperty(display,
                                   *window,
                                   propertyName,
                                   0,
                                   LONG_LENGTH,
                                   False,
                                   propertyType,
                                   &returnType,
                                   &returnFormat,
                                   &returnCount,
                                   &returnOffset,
                                   reinterpret_cast<unsigned char**>(&wmName)
                                  ) == Success) {
                if (returnType != propertyType) {
                    XFree(wmName);
                    wmName = nullptr;
                }
            }

            // attempt number 2 to get its name (plain version)
            if (!wmName) {
                propertyType = XA_STRING;
                if (XGetWindowProperty(display,
                                       *window,
                                       propertyName,
                                       0,
                                       LONG_LENGTH,
                                       False,
                                       propertyType,
                                       &returnType,
                                       &returnFormat,
                                       &returnCount,
                                       &returnOffset,
                                       reinterpret_cast<unsigned char**>(&wmName)
                                      ) == Success) {
                    if (returnType != propertyType) {
                        XFree(wmName);
                        wmName = nullptr;
                    }
                }
            }

            // HERE DO SOMETHING ABOUT IT
            std::fputs(wmName, stderr);

            // free the allocated data
            XFree(window);
            if (wmName)
                XFree(wmName);
        }
    }
}

void TrackOsDetails(std::stringstream &ss)
{

}

#endif // __linux

