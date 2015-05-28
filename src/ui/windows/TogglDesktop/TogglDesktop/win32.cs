using System;
using System.Drawing;
using System.Runtime.InteropServices;
using System.Text;

namespace TogglDesktop
{
    public class Win32
{
    public const int WM_MOUSEMOVE = 0x0200;
    public const int WM_LBUTTONDOWN = 0x0201;
    public const int WM_LBUTTONUP = 0x0202;
    public const int WM_RBUTTONDOWN = 0x0204;
    public const int WM_LBUTTONDBLCLK = 0x0203;

    public const int WM_MOUSELEAVE = 0x02A3;

    public const int WM_PAINT = 0x000F;
    public const int WM_ERASEBKGND = 0x0014;

    public const int WM_PRINT = 0x0317;

    public const int WM_HSCROLL = 0x0114;
    public const int WM_VSCROLL = 0x0115;

    public const int EM_GETSEL = 0x00B0;
    public const int EM_LINEINDEX = 0x00BB;
    public const int EM_LINEFROMCHAR = 0x00C9;

    public const int EM_POSFROMCHAR = 0x00D6;

    [DllImport("USER32.DLL", EntryPoint = "PostMessage")]
    public static extern bool PostMessage(IntPtr hwnd, uint msg,
                                          IntPtr wParam, IntPtr lParam);

    [DllImport("USER32.DLL", EntryPoint = "SendMessage")]
    public static extern int SendMessage(IntPtr hwnd, int msg, IntPtr wParam,
                                         IntPtr lParam);

    [DllImport("USER32.DLL", EntryPoint = "GetCaretBlinkTime")]
    public static extern uint GetCaretBlinkTime();

    const int WM_PRINTCLIENT = 0x0318;

    const long PRF_CHECKVISIBLE = 0x00000001L;
    const long PRF_NONCLIENT = 0x00000002L;
    const long PRF_CLIENT = 0x00000004L;
    const long PRF_ERASEBKGND = 0x00000008L;
    const long PRF_CHILDREN = 0x00000010L;
    const long PRF_OWNED = 0x00000020L;

    public static bool CaptureWindow(System.Windows.Forms.Control control,
                                     ref System.Drawing.Bitmap bitmap)
    {
        Graphics g2 = Graphics.FromImage(bitmap);

        int meint = (int)(PRF_CLIENT | PRF_ERASEBKGND);
        System.IntPtr meptr = new System.IntPtr(meint);

        System.IntPtr hdc = g2.GetHdc();
        Win32.SendMessage(control.Handle, Win32.WM_PRINT, hdc, meptr);

        g2.ReleaseHdc(hdc);
        g2.Dispose();

        return true;
    }

    [DllImport("user32.dll", SetLastError = true)]
    public static extern IntPtr FindWindow(string lpClassName, string lpWindowName);

    [DllImport("user32.dll")]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static extern bool EnumWindows(EnumWindowsProc lpEnumFunc, ref SearchData data);

    [DllImport("user32.dll", CharSet = CharSet.Auto, SetLastError = true)]
    public static extern int GetWindowText(IntPtr hWnd, StringBuilder lpString, int nMaxCount);

    [DllImport("User32")]
    public static extern int SetForegroundWindow(IntPtr hwnd);

    [DllImport("User32")]
    public static extern bool ShowWindow(IntPtr hwnd, int cmdshow);

    public const int SW_RESTORE = 9;

    public delegate bool EnumWindowsProc(IntPtr hWnd, ref SearchData data);

    public static bool EnumProc(IntPtr hWnd, ref SearchData data)
    {
        StringBuilder sb = new StringBuilder(1024);
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
        SearchData sd = new SearchData { Wndclass = wndclass, Title = title };
        EnumWindows(new EnumWindowsProc(EnumProc), ref sd);
        return sd.hWnd;
    }

    public class SearchData
    {
        public string Wndclass;
        public string Title;
        public IntPtr hWnd;
    }

    [DllImport("user32.dll")]
    public static extern int ReleaseCapture();

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

    [DllImport("user32", CallingConvention = CallingConvention.Winapi)]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static extern bool ShowScrollBar(IntPtr hwnd, int wBar, [MarshalAs(UnmanagedType.Bool)] bool bShow);

    [DllImport("user32.dll")]
    [return: MarshalAs(UnmanagedType.Bool)]
    public static extern bool SetWindowPos(IntPtr hWnd,
                                            int hWndInsertAfter, int x, int u, int cx, int cy, int uFlags);

    public const int HWND_TOPMOST = -1;
    public const int HWND_NOTOPMOST = -2;
    public const int SWP_NOMOVE = 0x0002;
    public const int SWP_NOSIZE = 0x0001;
    public const int SB_HORZ = 0;

    [DllImport("user32", CharSet = CharSet.Auto)]
    public static extern int GetWindowLong(IntPtr hwnd, int nIndex);
    public const int GWL_STYLE = (-16);
    public const int WS_VSCROLL = 0x200000;

    [DllImport("user32.dll")]
    public static extern bool RegisterHotKey(IntPtr hWnd, int id, uint fsModifiers, uint vk);

    [DllImport("user32.dll")]
    public static extern bool UnregisterHotKey(IntPtr hWnd, int id);

    public static int WM_HOTKEY = 0x0312;

    public const int HtBottom = 15;
}

}