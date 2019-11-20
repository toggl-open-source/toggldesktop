using System;
using System.Runtime.InteropServices;
// ReSharper disable InconsistentNaming

namespace TogglDesktop
{
static class Win32
{
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