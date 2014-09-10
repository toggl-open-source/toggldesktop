// Copyright 2014 Toggl Desktop developers.

#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")
#include <time.h>
#include <string>

static const int kFilenameBufferSize = 255;

template <class string_type>
inline typename string_type::value_type *writeInto(string_type *str,
        size_t length_with_null) {
    str->reserve(length_with_null);
    str->resize(length_with_null - 1);
    return &((*str)[0]);
}

int getFocusedWindowInfo(
    std::string *title,
    std::string *filename,
    bool *idle) {
    *title = "";
    *filename = "";
    *idle = false;

    // get window handle
    HWND window_handle = GetForegroundWindow();
    if (!window_handle) {
        *idle = true;
        return 0;
    }

    // get window title
    int length = GetWindowTextLength(window_handle) + 1;
    std::wstring title_wstring;
    GetWindowText(window_handle, writeInto(&title_wstring, length), length);
    std::string str(title_wstring.begin(), title_wstring.end());
    *title = str;

    // get process by window handle
    DWORD process_id;
    GetWindowThreadProcessId(window_handle, &process_id);

    // get the filename of another process
    HANDLE ps = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE,
                            process_id);

    CHAR filename_buffer[kFilenameBufferSize];
    if (GetModuleFileNameExA(ps, 0, filename_buffer, kFilenameBufferSize) > 0) {
        *filename = std::string(filename_buffer);
    }

    CloseHandle(ps);

    return 0;
}
