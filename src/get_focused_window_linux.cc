// Copyright 2014 Toggl Desktop developers.

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include <cstring>
#include <string>
#include <typeinfo>

#include "./get_focused_window.h"

#define HANDLE_EINTR(x) ({ \
  __typeof__(x) __eintr_result__; \
  do { \
    __eintr_result__ = x; \
  } while (__eintr_result__ == -1 && errno == EINTR); \
  __eintr_result__;\
})

static const char kNetActiveWindow[] = "_NET_ACTIVE_WINDOW";
static const int kMaxPropertyValueLen = 4096;

static char *get_property(Display *disp, Window win, Atom xa_prop_type,
                          const char *prop_name, unsigned long *size) { // NOLINT
    Atom xa_prop_name;
    Atom xa_ret_type;
    int ret_format;
    unsigned long ret_nitems; // NOLINT
    unsigned long ret_bytes_after; // NOLINT
    unsigned long tmp_size; // NOLINT
    unsigned char *ret_prop;
    char *ret;

    xa_prop_name = XInternAtom(disp, prop_name, False);

    // kMaxPropertyValueLen / 4 explanation (XGetWindowProperty manpage):
    // long_length = Specifies the length in 32-bit multiples of the data
    // to be retrieved.
    if (XGetWindowProperty(disp,
                           win,
                           xa_prop_name,
                           0,
                           kMaxPropertyValueLen / 4,
                           False,
                           xa_prop_type,
                           &xa_ret_type,
                           &ret_format,
                           &ret_nitems,
                           &ret_bytes_after,
                           &ret_prop) != Success) {
        return nullptr;
    }

    if (xa_ret_type != xa_prop_type) {
        XFree(ret_prop);
        return nullptr;
    }

    // null terminate the result to make string handling easier
    tmp_size = (static_cast<unsigned>(ret_format) / (32 / sizeof(long))) * ret_nitems;
    ret = reinterpret_cast<char *>(malloc(tmp_size + 1));
    if (!ret) {
        XFree(ret_prop);
        return nullptr;
    }
    memcpy(ret, ret_prop, tmp_size);
    ret[tmp_size] = '\0';

    if (size) {
        *size = tmp_size;
    }

    XFree(ret_prop);
    return ret;
}

int getFocusedWindowInfo(
    std::string *title,
    std::string *filename,
    bool *idle) {
    *title = "";
    *filename = "";
    *idle = false;

    Display *display = XOpenDisplay(nullptr);
    if (!display) {
        return 1;
    }

    // get active window
    unsigned long size = 0; // NOLINT
    Window active_window = 0;
    char *prop = get_property(
        display,
        DefaultRootWindow(display),
        XA_WINDOW,
        kNetActiveWindow,
        &size);
    if (prop) {
        active_window = *(reinterpret_cast<Window *>(prop));
    }
    free(prop);

    // get title of active window
    if (active_window) {
        char *net_wm_name = get_property(
            display,
            active_window,
            XInternAtom(display, "UTF8_STRING", false),
            "_NET_WM_NAME",
            nullptr);
        if (net_wm_name) {
            *title = std::string(net_wm_name);
        } else {
            char *wm_name = get_property(display, active_window,
                                         XA_STRING, "WM_NAME", nullptr);
            if (wm_name) {
                *title = std::string(wm_name);
            }
            free(wm_name);
        }
        free(net_wm_name);
    }

    // get pid of active window
    unsigned long *pid = nullptr;
    if (active_window) {
        pid = reinterpret_cast<unsigned long*>(get_property(display, active_window,
                                                            XA_CARDINAL, "_NET_WM_PID", nullptr));
        if (pid) {
            // get process name by pid
            char buf[256];
            snprintf(buf, sizeof(buf), "/proc/%lu/stat", *pid);
            const int fd = open(buf, O_RDONLY);
            if (fd >= 0) {
                const ssize_t len =
                    HANDLE_EINTR(read(fd, buf, sizeof(buf) - 1));
                HANDLE_EINTR(close(fd));
                if (len > 0) {
                    buf[len] = 0;
                    // The start of the file looks like:
                    //   <pid> (<name>) R <parent pid>
                    unsigned tmp_pid, tmp_ppid;
                    char *process_name = nullptr;
                    if (sscanf(buf, "%u (%m[^)]) %*c %u", // NOLINT
                               &tmp_pid, &process_name, &tmp_ppid) == 3) {
                        *filename = std::string(process_name);
                    }
                    free(process_name);
                }
            }
        }
        free(pid);
    }

    // Must close display, else max window client count is reached quickly
    XCloseDisplay(display);

    return 0;
}
