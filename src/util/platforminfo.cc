#include "platforminfo.h"

#if defined(__linux)

#include <cstdio>
#include <fstream>

#include <X11/Xlib.h>
#include <X11/Xmu/WinUtil.h>
#include <X11/Xatom.h>

#define LONG_LENGTH 512

std::string getWMName() {
    Display *display { nullptr };
    Window *window { nullptr };
    Atom propertyName { 0 };
    Atom propertyType { 0 };
    Atom returnType { 0 };
    int returnFormat { 0 };
    unsigned long returnCount { 0 };
    unsigned long returnOffset { 0 };
    char *wmName { nullptr };
    std::string result;

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

            // success
            result = std::string(wmName);

            // free the allocated data
            XFree(window);
            if (wmName)
                XFree(wmName);
        }
    }
    return result;
}

std::string getDEName() {
    char *env = getenv("XDG_CURRENT_DESKTOP");
    return env ? std::string(env) : "";
}

std::string getDistroName() {
    std::string name, version;
    std::ifstream osinfo("/etc/os-release", std::ifstream::in);
    while (osinfo.good() && !osinfo.eof()) {
        std::string line;
        std::getline(osinfo, line);
        if (line.find("NAME=") == 0)
            return line.substr(5);
    }
    return std::string();
}

std::string getDistroVersion() {
    std::string name, version;
    std::ifstream osinfo("/etc/os-release", std::ifstream::in);
    while (osinfo.good() && !osinfo.eof()) {
        std::string line;
        std::getline(osinfo, line);
        if (line.find("VERSION_ID=") == 0)
            return line.substr(11);
    }
    return std::string();
}

std::string getSessionType() {
    char *val = getenv("XDG_SESSION_TYPE");
    if (val && *val)
        return val;
    return std::string();
}

void RetrieveOsDetails(std::stringstream &ss) {
    std::string wm = getWMName();
    if (!wm.empty()) {
        ss << "osdetails/wm-" << wm << ";";
    }
    std::string de = getDEName();
    if (!de.empty()) {
        ss << "osdetails/de-" << de << ";";
    }
    std::string distro = getDistroName() + getDistroVersion();
    if (!distro.empty()) {
        ss << "osdetails/distro-" << distro << ";";
    }
    std::string sessionType = getSessionType();
    if (!sessionType.empty()) {
        ss << "osdetails/session-" << sessionType << ";";
    }
    ss << "osdetails/build-" << TOGGL_BUILD_TYPE;
}

std::map<std::string, std::string> RetrieveOsDetailsMap() {
    std::map<std::string, std::string> result;
    std::string wm = getWMName();
    if (!wm.empty()) {
        result["window_manager"] = wm;
    }
    std::string de = getDEName();
    if (!de.empty()) {
        result["desktop_environment"] = de;
    }
    std::string distro = getDistroName();
    if (!distro.empty()) {
        result["distribution"] = distro;
    }
    std::string version = getDistroVersion();
    if (!version.empty()) {
        result["distribution_version"] = version;
    }
    std::string sessionType = getSessionType();
    if (!sessionType.empty()) {
        result["session_type"] = sessionType;
    }
    result["build_type"] = TOGGL_BUILD_TYPE;
    return result;
}

#endif // __linux

