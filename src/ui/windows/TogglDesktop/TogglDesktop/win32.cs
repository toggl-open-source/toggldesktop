using System;
using System.Runtime.InteropServices;
using System.Text;
// ReSharper disable InconsistentNaming

namespace TogglDesktop
{
static class Win32
{
    public const int WM_MOUSEMOVE = 0x0200;
    public const int WM_LBUTTONDOWN = 0x0201;
    public const int WM_LBUTTONUP = 0x0202;
    public const int WM_RBUTTONDOWN = 0x0204;
    public const int WM_LBUTTONDBLCLK = 0x0203;

    public const int WM_MOUSELEAVE = 0x02A3;

    public const int WM_PAINT = 0x000F;

    public const int WM_HSCROLL = 0x0114;
    public const int WM_VSCROLL = 0x0115;

    public const int EM_POSFROMCHAR = 0x00D6;

    [DllImport("user32.dll")]
    [return: MarshalAs(UnmanagedType.Bool)]
    private static extern bool EnumWindows(EnumWindowsProc lpEnumFunc, ref SearchData data);

    [DllImport("user32.dll", CharSet = CharSet.Auto, SetLastError = true)]
    private static extern int GetWindowText(IntPtr hWnd, StringBuilder lpString, int nMaxCount);

    [DllImport("User32")]
    public static extern int SetForegroundWindow(IntPtr hwnd);

    [DllImport("User32")]
    public static extern bool ShowWindow(IntPtr hwnd, int cmdshow);

    public const int SW_RESTORE = 9;

    private delegate bool EnumWindowsProc(IntPtr hWnd, ref SearchData data);

    private static bool EnumProc(IntPtr hWnd, ref SearchData data)
    {
        var sb = new StringBuilder(1024);
        GetWindowText(hWnd, sb, sb.Capacity);
        if (sb.ToString().Contains(data.Title))
        {
            data.hWnd = hWnd;
            return false;    // Found the window
        }
        return true;
    }

    [DllImport("kernel32.dll")]
    public static extern bool AttachConsole(int dwProcessId);

    public const int ATTACH_PARENT_PROCESS = -1;

    public static IntPtr SearchForWindow(string wndclass, string title)
    {
        var sd = new SearchData { Wndclass = wndclass, Title = title };
        EnumWindows(EnumProc, ref sd);
        return sd.hWnd;
    }

    private class SearchData
    {
        public string Wndclass;
        public string Title;
        public IntPtr hWnd;
    }

    [DllImport("user32.dll")]
    public static extern int SendMessage(IntPtr hwnd, int msg, int wparam, int lparam);

    public const int wmNcLButtonDown = 0xA1;
    public const int wmNcLButtonUp = 0xA2;
    public const int HtBottomRight = 17;

    [StructLayout(LayoutKind.Sequential)]
    public struct LASTINPUTINFO
    {
        public static readonly int SizeOf =
            Marshal.SizeOf(typeof(LASTINPUTINFO));

        [MarshalAs(UnmanagedType.U4)]
        public int cbSize;

        [MarshalAs(UnmanagedType.U4)]
        public int dwTime;
    }

    [DllImport("user32.dll")]
    public static extern bool GetLastInputInfo(out LASTINPUTINFO plii);

    [DllImport("user32.dll")]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static extern bool SetWindowPos(IntPtr hWnd,
                                           int hWndInsertAfter, int x, int u, int cx, int cy, int uFlags);

    public const int HWND_TOPMOST = -1;
    public const int HWND_NOTOPMOST = -2;
    public const int SWP_NOMOVE = 0x0002;
    public const int SWP_NOSIZE = 0x0001;

    [DllImport("user32", CharSet = CharSet.Auto)]
    public static extern int GetWindowLong(IntPtr hwnd, int nIndex);
    public const int GWL_STYLE = (-16);
    public const int WS_VSCROLL = 0x200000;

    [DllImport("user32.dll")]
    public static extern bool RegisterHotKey(IntPtr hWnd, int id, uint fsModifiers, uint vk);

    [DllImport("user32.dll")]
    public static extern bool UnregisterHotKey(IntPtr hWnd, int id);

    public const int WM_HOTKEY = 0x0312;

    [StructLayout(LayoutKind.Sequential)]
    public struct Rectangle
    {
        public readonly int Left;
        public readonly int Top;
        public readonly int Right;
        public readonly int Bottom;
    }
    [DllImport("user32.dll")]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static extern bool GetWindowRect(IntPtr hWnd, out Rectangle lpRect);
}

}