using System;
using System.Runtime.InteropServices;
using System.Text;
// ReSharper disable InconsistentNaming

namespace TogglDesktop
{
static class Win32
{
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

    
    [DllImport("user32.dll")]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static extern bool GetCursorPos(ref Win32Point pt);

    [StructLayout(LayoutKind.Sequential)]
    public struct Win32Point
    {
        public Int32 X;
        public Int32 Y;
    }
    const int WS_EX_TRANSPARENT = 0x00000020;
    const int GWL_EXSTYLE = (-20);

    [DllImport("user32.dll")]
    static extern int GetWindowLong(IntPtr hwnd, int index);

    [DllImport("user32.dll")]
    static extern int SetWindowLong(IntPtr hwnd, int index, int newStyle);

    public static void SetWindowExTransparent(IntPtr hwnd)
    {
        var extendedStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
        SetWindowLong(hwnd, GWL_EXSTYLE, extendedStyle | WS_EX_TRANSPARENT);
    }
}

}