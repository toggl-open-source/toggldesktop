#region Using directives

using System;
using System.Windows.Forms;
using System.Drawing;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Security.Permissions;
using System.Text;
using System.Threading;

#endregion

namespace Kobush.Windows.Forms
{
	[System.Security.SuppressUnmanagedCodeSecurity]
	[System.Runtime.InteropServices.ComVisible(false)]
	internal sealed class NativeMethods
	{
		private NativeMethods()
		{ }

		#region WindowStyle
		[Flags]
		public enum WindowStyle
		{
			WS_OVERLAPPED = 0x00000000,
			WS_POPUP = -2147483648, //0x80000000,
			WS_CHILD = 0x40000000,
			WS_MINIMIZE = 0x20000000,
			WS_VISIBLE = 0x10000000,
			WS_DISABLED = 0x08000000,
			WS_CLIPSIBLINGS = 0x04000000,
			WS_CLIPCHILDREN = 0x02000000,
			WS_MAXIMIZE = 0x01000000,
			WS_CAPTION = 0x00C00000,
			WS_BORDER = 0x00800000,
			WS_DLGFRAME = 0x00400000,
			WS_VSCROLL = 0x00200000,
			WS_HSCROLL = 0x00100000,
			WS_SYSMENU = 0x00080000,
			WS_THICKFRAME = 0x00040000,
			WS_GROUP = 0x00020000,
			WS_TABSTOP = 0x00010000,
			WS_MINIMIZEBOX = 0x00020000,
			WS_MAXIMIZEBOX = 0x00010000,
			WS_TILED = WS_OVERLAPPED,
			WS_ICONIC = WS_MINIMIZE,
			WS_SIZEBOX = WS_THICKFRAME,
			WS_TILEDWINDOW = WS_OVERLAPPEDWINDOW,
			WS_OVERLAPPEDWINDOW = (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU |
									WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX),
			WS_POPUPWINDOW = (WS_POPUP | WS_BORDER | WS_SYSMENU),
			WS_CHILDWINDOW = (WS_CHILD)
		}
		#endregion //WindowStyle

		#region WindowStyleEx
		[Flags]
		public enum WindowStyleEx
		{
			WS_EX_DLGMODALFRAME = 0x00000001,
			WS_EX_NOPARENTNOTIFY = 0x00000004,
			WS_EX_TOPMOST = 0x00000008,
			WS_EX_ACCEPTFILES = 0x00000010,
			WS_EX_TRANSPARENT = 0x00000020,
			WS_EX_MDICHILD = 0x00000040,
			WS_EX_TOOLWINDOW = 0x00000080,
			WS_EX_WINDOWEDGE = 0x00000100,
			WS_EX_CLIENTEDGE = 0x00000200,
			WS_EX_CONTEXTHELP = 0x00000400,
			WS_EX_RIGHT = 0x00001000,
			WS_EX_LEFT = 0x00000000,
			WS_EX_RTLREADING = 0x00002000,
			WS_EX_LTRREADING = 0x00000000,
			WS_EX_LEFTSCROLLBAR = 0x00004000,
			WS_EX_RIGHTSCROLLBAR = 0x00000000,
			WS_EX_CONTROLPARENT = 0x00010000,
			WS_EX_STATICEDGE = 0x00020000,
			WS_EX_APPWINDOW = 0x00040000,
			WS_EX_OVERLAPPEDWINDOW = (WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE),
			WS_EX_PALETTEWINDOW = (WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST),
			WS_EX_LAYERED = 0x00080000,
			WS_EX_NOINHERITLAYOUT = 0x00100000, // Disable inheritence of mirroring by children
			WS_EX_LAYOUTRTL = 0x00400000, // Right to left mirroring
			WS_EX_COMPOSITED = 0x02000000,
			WS_EX_NOACTIVATE = 0x08000000,
		}
		#endregion //WindowStyleEx

        #region StaticStyle
        [Flags]
        public enum StaticStyle
        {
		    SS_ETCHEDHORZ = 0x00000010,
	        SS_ETCHEDVERT = 0x00000011,
        }
        #endregion

        #region WindowMessages
        public enum WindowMessages
		{
			WM_NULL = 0x0000,
			WM_CREATE = 0x0001,
			WM_DESTROY = 0x0002,
			WM_MOVE = 0x0003,
			WM_SIZE = 0x0005,
			WM_ACTIVATE = 0x0006,
			WM_SETFOCUS = 0x0007,
			WM_KILLFOCUS = 0x0008,
			WM_ENABLE = 0x000A,
			WM_SETREDRAW = 0x000B,
			WM_SETTEXT = 0x000C,
			WM_GETTEXT = 0x000D,
			WM_GETTEXTLENGTH = 0x000E,
			WM_PAINT = 0x000F,
			WM_CLOSE = 0x0010,

			WM_QUIT = 0x0012,
			WM_ERASEBKGND = 0x0014,
			WM_SYSCOLORCHANGE = 0x0015,
			WM_SHOWWINDOW = 0x0018,

			WM_ACTIVATEAPP = 0x001C,

			WM_SETCURSOR = 0x0020,
			WM_MOUSEACTIVATE = 0x0021,
			WM_GETMINMAXINFO = 0x24,
			WM_WINDOWPOSCHANGING = 0x0046,
			WM_WINDOWPOSCHANGED = 0x0047,

			WM_CONTEXTMENU = 0x007B,
			WM_STYLECHANGING = 0x007C,
			WM_STYLECHANGED = 0x007D,
			WM_DISPLAYCHANGE = 0x007E,
			WM_GETICON = 0x007F,
			WM_SETICON = 0x0080,

			// non client area
			WM_NCCREATE = 0x0081,
			WM_NCDESTROY = 0x0082,
			WM_NCCALCSIZE = 0x0083,
			WM_NCHITTEST = 0x84,
			WM_NCPAINT = 0x0085,
			WM_NCACTIVATE = 0x0086,

			WM_GETDLGCODE = 0x0087,

			WM_SYNCPAINT = 0x0088,

			// non client mouse
			WM_NCMOUSEMOVE = 0x00A0,
			WM_NCLBUTTONDOWN = 0x00A1,
			WM_NCLBUTTONUP = 0x00A2,
			WM_NCLBUTTONDBLCLK = 0x00A3,
			WM_NCRBUTTONDOWN = 0x00A4,
			WM_NCRBUTTONUP = 0x00A5,
			WM_NCRBUTTONDBLCLK = 0x00A6,
			WM_NCMBUTTONDOWN = 0x00A7,
			WM_NCMBUTTONUP = 0x00A8,
			WM_NCMBUTTONDBLCLK = 0x00A9,

			// keyboard
			WM_KEYDOWN = 0x0100,
			WM_KEYUP = 0x0101,
			WM_CHAR = 0x0102,

			WM_SYSCOMMAND = 0x0112,

            // menu
            WM_INITMENU = 0x0116,
            WM_INITMENUPOPUP = 0x0117,
            WM_MENUSELECT = 0x011F,
            WM_MENUCHAR = 0x0120,
            WM_ENTERIDLE = 0x0121,
            WM_MENURBUTTONUP = 0x0122,
            WM_MENUDRAG = 0x0123,
            WM_MENUGETOBJECT = 0x0124,
            WM_UNINITMENUPOPUP = 0x0125,
            WM_MENUCOMMAND  = 0x0126,

			WM_CHANGEUISTATE = 0x0127,
			WM_UPDATEUISTATE = 0x0128,
			WM_QUERYUISTATE = 0x0129,

			// mouse
			WM_MOUSEFIRST = 0x0200,
			WM_MOUSEMOVE = 0x0200,
			WM_LBUTTONDOWN = 0x0201,
			WM_LBUTTONUP = 0x0202,
			WM_LBUTTONDBLCLK = 0x0203,
			WM_RBUTTONDOWN = 0x0204,
			WM_RBUTTONUP = 0x0205,
			WM_RBUTTONDBLCLK = 0x0206,
			WM_MBUTTONDOWN = 0x0207,
			WM_MBUTTONUP = 0x0208,
			WM_MBUTTONDBLCLK = 0x0209,
			WM_MOUSEWHEEL = 0x020A,
			WM_MOUSELAST = 0x020D,

			WM_PARENTNOTIFY = 0x0210,
			WM_ENTERMENULOOP = 0x0211,
			WM_EXITMENULOOP = 0x0212,

			WM_NEXTMENU = 0x0213,
			WM_SIZING = 0x0214,
			WM_CAPTURECHANGED = 0x0215,
			WM_MOVING = 0x0216,

			WM_ENTERSIZEMOVE = 0x0231,
			WM_EXITSIZEMOVE = 0x0232,

			WM_MOUSELEAVE = 0x02A3,
			WM_MOUSEHOVER = 0x02A1,
			WM_NCMOUSEHOVER = 0x02A0,
			WM_NCMOUSELEAVE = 0x02A2,

			WM_MDIACTIVATE = 0x0222,
			WM_HSCROLL = 0x0114,
			WM_VSCROLL = 0x0115,

			WM_PRINT = 0x0317,
			WM_PRINTCLIENT = 0x0318,
		}
		#endregion //WindowMessages

		#region SystemCommands

		public enum SystemCommands
		{
			SC_SIZE = 0xF000,
			SC_MOVE = 0xF010,
			SC_MINIMIZE = 0xF020,
			SC_MAXIMIZE = 0xF030,
			SC_MAXIMIZE2 = 0xF032,	// fired from double-click on caption
			SC_NEXTWINDOW = 0xF040,
			SC_PREVWINDOW = 0xF050,
			SC_CLOSE = 0xF060,
			SC_VSCROLL = 0xF070,
			SC_HSCROLL = 0xF080,
			SC_MOUSEMENU = 0xF090,
			SC_KEYMENU = 0xF100,
			SC_ARRANGE = 0xF110,
			SC_RESTORE = 0xF120,
			SC_RESTORE2 = 0xF122,	// fired from double-click on caption
			SC_TASKLIST = 0xF130,
			SC_SCREENSAVE = 0xF140,
			SC_HOTKEY = 0xF150,

			SC_DEFAULT = 0xF160,
			SC_MONITORPOWER = 0xF170,
			SC_CONTEXTHELP = 0xF180,
			SC_SEPARATOR = 0xF00F
		}

		#endregion // SystemCommands

		#region PeekMessageOptions
		[Flags]
		public enum PeekMessageOptions
		{
			PM_NOREMOVE = 0x0000,
			PM_REMOVE = 0x0001,
			PM_NOYIELD = 0x0002
		}
		#endregion // PeekMessageOptions

		#region NCHITTEST enum
		/// <summary>
		/// Location of cursor hot spot returnet in WM_NCHITTEST.
		/// </summary>
		public enum NCHITTEST
		{
			/// <summary>
			/// On the screen background or on a dividing line between windows 
			/// (same as HTNOWHERE, except that the DefWindowProc function produces a system beep to indicate an error).
			/// </summary>
			HTERROR = (-2),
			/// <summary>
			/// In a window currently covered by another window in the same thread 
			/// (the message will be sent to underlying windows in the same thread until one of them returns a code that is not HTTRANSPARENT).
			/// </summary>
			HTTRANSPARENT = (-1),
			/// <summary>
			/// On the screen background or on a dividing line between windows.
			/// </summary>
			HTNOWHERE = 0,
			/// <summary>In a client area.</summary>
			HTCLIENT = 1,
			/// <summary>In a title bar.</summary>
			HTCAPTION = 2,
			/// <summary>In a window menu or in a Close button in a child window.</summary>
			HTSYSMENU = 3,
			/// <summary>In a size box (same as HTSIZE).</summary>
			HTGROWBOX = 4,
			/// <summary>In a menu.</summary>
			HTMENU = 5,
			/// <summary>In a horizontal scroll bar.</summary>
			HTHSCROLL = 6,
			/// <summary>In the vertical scroll bar.</summary>
			HTVSCROLL = 7,
			/// <summary>In a Minimize button.</summary>
			HTMINBUTTON = 8,
			/// <summary>In a Maximize button.</summary>
			HTMAXBUTTON = 9,
			/// <summary>In the left border of a resizable window 
			/// (the user can click the mouse to resize the window horizontally).</summary>
			HTLEFT = 10,
			/// <summary>
			/// In the right border of a resizable window 
			/// (the user can click the mouse to resize the window horizontally).
			/// </summary>
			HTRIGHT = 11,
			/// <summary>In the upper-horizontal border of a window.</summary>
			HTTOP = 12,
			/// <summary>In the upper-left corner of a window border.</summary>
			HTTOPLEFT = 13,
			/// <summary>In the upper-right corner of a window border.</summary>
			HTTOPRIGHT = 14,
			/// <summary>	In the lower-horizontal border of a resizable window 
			/// (the user can click the mouse to resize the window vertically).</summary>
			HTBOTTOM = 15,
			/// <summary>In the lower-left corner of a border of a resizable window 
			/// (the user can click the mouse to resize the window diagonally).</summary>
			HTBOTTOMLEFT = 16,
			/// <summary>	In the lower-right corner of a border of a resizable window 
			/// (the user can click the mouse to resize the window diagonally).</summary>
			HTBOTTOMRIGHT = 17,
			/// <summary>In the border of a window that does not have a sizing border.</summary>
			HTBORDER = 18,

			HTOBJECT = 19,
			/// <summary>In a Close button.</summary>
			HTCLOSE = 20,
			/// <summary>In a Help button.</summary>
			HTHELP = 21,
		}

		#endregion //NCHITTEST

		#region DCX enum
		[Flags()]
		internal enum DCX
		{
			DCX_CACHE = 0x2,
			DCX_CLIPCHILDREN = 0x8,
			DCX_CLIPSIBLINGS = 0x10,
			DCX_EXCLUDERGN = 0x40,
			DCX_EXCLUDEUPDATE = 0x100,
			DCX_INTERSECTRGN = 0x80,
			DCX_INTERSECTUPDATE = 0x200,
			DCX_LOCKWINDOWUPDATE = 0x400,
			DCX_NORECOMPUTE = 0x100000,
			DCX_NORESETATTRS = 0x4,
			DCX_PARENTCLIP = 0x20,
			DCX_VALIDATE = 0x200000,
			DCX_WINDOW = 0x1,
		}
		#endregion //DCX

		#region ShowWindow flags
		[Flags]
		public enum ShowWindowOptions
		{
			SW_HIDE = 0,
			SW_SHOWNOACTIVATE = 4,
		}

		#endregion

		#region SetWindowPosition flags
		[Flags]
		public enum SetWindowPosOptions
		{
			SWP_NOSIZE = 0x0001,
			SWP_NOMOVE = 0x0002,
			SWP_NOZORDER = 0x0004,
			SWP_NOACTIVATE = 0x0010,
			SWP_FRAMECHANGED = 0x0020,	/* The frame changed: send WM_NCCALCSIZE */
			SWP_SHOWWINDOW = 0x0040,
			SWP_HIDEWINDOW = 0x0080,
			SWP_NOCOPYBITS = 0x0100,
			SWP_NOOWNERZORDER = 0x0200,	/* Don't do owner Z ordering */
			SWP_NOSENDCHANGING = 0x0400		/* Don't send WM_WINDOWPOSCHANGING */
		}
		#endregion

		#region RedrawWindow flags
		[Flags]
		public enum RedrawWindowOptions
		{
			RDW_INVALIDATE = 0x0001,
			RDW_INTERNALPAINT = 0x0002,
			RDW_ERASE = 0x0004,
			RDW_VALIDATE = 0x0008,
			RDW_NOINTERNALPAINT = 0x0010,
			RDW_NOERASE = 0x0020,
			RDW_NOCHILDREN = 0x0040,
			RDW_ALLCHILDREN = 0x0080,
			RDW_UPDATENOW = 0x0100,
			RDW_ERASENOW = 0x0200,
			RDW_FRAME = 0x0400,
			RDW_NOFRAME = 0x0800
		}
		#endregion

		#region RECT structure

		[StructLayout(LayoutKind.Sequential)]
		internal struct RECT
		{
			public int left;
			public int top;
			public int right;
			public int bottom;

			public RECT(int left, int top, int right, int bottom)
			{
				this.left = left;
				this.top = top;
				this.right = right;
				this.bottom = bottom;
			}

			public Rectangle Rect { get { return new Rectangle(this.left, this.top, this.right - this.left, this.bottom - this.top); } }

			public static RECT FromXYWH(int x, int y, int width, int height)
			{
				return new RECT(x,
								y,
								x + width,
								y + height);
			}

			public static RECT FromRectangle(Rectangle rect)
			{
				return new RECT(rect.Left,
								 rect.Top,
								 rect.Right,
								 rect.Bottom);
			}
		}

		#endregion RECT structure

		#region WINDOWPOS
		[StructLayout(LayoutKind.Sequential)]
		internal struct WINDOWPOS
		{
			internal IntPtr hwnd;
			internal IntPtr hWndInsertAfter;
			internal int x;
			internal int y;
			internal int cx;
			internal int cy;
			internal uint flags;
		}
		#endregion //WINDOWPOS

		#region NCCALCSIZE_PARAMS
		//http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winui/winui/windowsuserinterface/windowing/windows/windowreference/windowstructures/nccalcsize_params.asp
		[StructLayout(LayoutKind.Sequential)]
		public struct NCCALCSIZE_PARAMS
		{
			/// <summary>
			/// Contains the new coordinates of a window that has been moved or resized, that is, it is the proposed new window coordinates.
			/// </summary>
			public RECT rectProposed;
			/// <summary>
			/// Contains the coordinates of the window before it was moved or resized.
			/// </summary>
			public RECT rectBeforeMove;
			/// <summary>
			/// Contains the coordinates of the window's client area before the window was moved or resized.
			/// </summary>
			public RECT rectClientBeforeMove;
			/// <summary>
			/// Pointer to a WINDOWPOS structure that contains the size and position values specified in the operation that moved or resized the window.
			/// </summary>
			public WINDOWPOS lpPos;
		}
		#endregion //NCCALCSIZE_PARAMS

		#region TRACKMOUSEEVENT structure

		[StructLayout(LayoutKind.Sequential)]
		public class TRACKMOUSEEVENT
		{
			public TRACKMOUSEEVENT()
			{
				this.cbSize = Marshal.SizeOf(typeof(NativeMethods.TRACKMOUSEEVENT));
				this.dwHoverTime = 100;
			}

			public int cbSize;
			public int dwFlags;
			public IntPtr hwndTrack;
			public int dwHoverTime;
		}

		#endregion

		#region TrackMouseEventFalgs enum

		[Flags]
		public enum TrackMouseEventFalgs
		{
			TME_HOVER = 1,
			TME_LEAVE = 2,
			TME_NONCLIENT = 0x00000010
		}

		#endregion

		public enum TernaryRasterOperations
		{
			SRCCOPY = 0x00CC0020, /* dest = source*/
			SRCPAINT = 0x00EE0086, /* dest = source OR dest*/
			SRCAND = 0x008800C6, /* dest = source AND dest*/
			SRCINVERT = 0x00660046, /* dest = source XOR dest*/
			SRCERASE = 0x00440328, /* dest = source AND (NOT dest )*/
			NOTSRCCOPY = 0x00330008, /* dest = (NOT source)*/
			NOTSRCERASE = 0x001100A6, /* dest = (NOT src) AND (NOT dest) */
			MERGECOPY = 0x00C000CA, /* dest = (source AND pattern)*/
			MERGEPAINT = 0x00BB0226, /* dest = (NOT source) OR dest*/
			PATCOPY = 0x00F00021, /* dest = pattern*/
			PATPAINT = 0x00FB0A09, /* dest = DPSnoo*/
			PATINVERT = 0x005A0049, /* dest = pattern XOR dest*/
			DSTINVERT = 0x00550009, /* dest = (NOT dest)*/
			BLACKNESS = 0x00000042, /* dest = BLACK*/
			WHITENESS = 0x00FF0062, /* dest = WHITE*/
		};

		public static readonly IntPtr TRUE = new IntPtr(1);
		public static readonly IntPtr FALSE = new IntPtr(0);

		public static readonly IntPtr HWND_TOPMOST = new IntPtr(-1);

		[DllImport("user32.dll")]
		public static extern IntPtr GetDesktopWindow();

		[DllImport("user32.dll")]
		public static extern IntPtr SetParent(IntPtr hWndChild, IntPtr hParent);

		[DllImport("user32.dll")]
		public static extern Int32 SetWindowLong(IntPtr hWnd, Int32 Offset, Int32 newLong);

		[DllImport("user32.dll")]
		public static extern Int32 GetWindowLong(IntPtr hWnd, Int32 Offset);

		[DllImport("user32.dll")]
		public static extern Int32 ShowWindow(IntPtr hWnd, Int32 dwFlags);

		[DllImport("user32.dll")]
		public static extern Int32 SetWindowPos(IntPtr hWnd, IntPtr hWndAfter, Int32 x, Int32 y, Int32 cx, Int32 cy, UInt32 uFlags);

		[DllImport("user32.dll")]
		public static extern int SendMessage(IntPtr hwnd, int wMsg, IntPtr wParam, IntPtr lParam);

		[DllImport("user32.dll")]
		public static extern bool PeekMessage(ref Message msg, IntPtr hwnd, int msgMin, int msgMax, int remove);

		[DllImport("user32.dll")]
		public static extern bool SetWindowPos(IntPtr hWnd, IntPtr hWndInsertAfter,
			int x, int y, int cx, int cy, int flags);

		[DllImport("user32.dll")]
		public static extern bool RedrawWindow(IntPtr hWnd, IntPtr rectUpdate, IntPtr hrgnUpdate, uint flags);

		[DllImport("user32.dll")]
		public static extern IntPtr GetDCEx(IntPtr hwnd, IntPtr hrgnclip, uint fdwOptions);

        [DllImport("user32.dll")]
        public static extern IntPtr GetWindowDC(IntPtr hwnd);

        [DllImport("user32.dll")]
        public static extern int ReleaseDC(IntPtr hwnd, IntPtr hDC);

		[DllImport("user32.dll")]
		public static extern int GetWindowRect(IntPtr hwnd, ref RECT lpRect);

		[DllImport("user32.dll")]
		public static extern void DisableProcessWindowsGhosting();

        [DllImport("user32.dll")]
        public static extern short GetAsyncKeyState(int nVirtKey);

        public const int VK_LBUTTON = 0x01;
        public const int VK_RBUTTON = 0x02;

		[DllImport("uxtheme.dll")]
		public static extern int SetWindowTheme(IntPtr hwnd, String pszSubAppName,
										 String pszSubIdList);

		[DllImport("comctl32.dll", SetLastError = true)]
		private static extern bool _TrackMouseEvent(TRACKMOUSEEVENT tme);

		public static bool TrackMouseEvent(TRACKMOUSEEVENT tme)
		{
			return _TrackMouseEvent(tme);
		}

        public static int GetLastError()
        {
            return System.Runtime.InteropServices.Marshal.GetLastWin32Error();
        }

		[DllImport("gdi32.dll")]
		public static extern IntPtr CreateCompatibleDC(IntPtr hDC);

		[DllImport("gdi32.dll")]
		public static extern IntPtr CreateCompatibleBitmap(IntPtr hDC, int nWidth, int nHeight);

		[DllImport("gdi32.dll")]
		public static extern IntPtr SelectObject(IntPtr hDC, IntPtr hObject);

		[DllImport("gdi32.dll")]
		public static extern bool BitBlt(IntPtr hObject, int nXDest, int nYDest, int nWidth,
		   int nHeight, IntPtr hObjSource, int nXSrc, int nYSrc, TernaryRasterOperations dwRop);

		[DllImport("gdi32.dll")]
		public static extern bool DeleteObject(IntPtr hObject);

		[DllImport("gdi32.dll")]
		public static extern bool DeleteDC(IntPtr hDC);
 
		#region AppBarInfo
		
		[StructLayout(LayoutKind.Sequential)]
		public struct APPBARDATA
		{
			public System.UInt32 cbSize;
			public System.IntPtr hWnd;
			public System.UInt32 uCallbackMessage;
			public System.UInt32 uEdge;
			public RECT rc;
			public System.Int32 lParam;
		}

		[DllImport("user32.dll")]
		public static extern System.IntPtr FindWindow(String lpClassName, String lpWindowName);

		[DllImport("shell32.dll")]
		public static extern System.UInt32 SHAppBarMessage(System.UInt32 dwMessage, ref APPBARDATA data);

		[DllImport("user32.dll")]
		public static extern System.Int32 SystemParametersInfo(System.UInt32 uiAction, System.UInt32 uiParam,
			System.IntPtr pvParam, System.UInt32 fWinIni);


		public class AppBarInfo
		{
			private APPBARDATA m_data;

			// Appbar messages
			private const int ABM_NEW = 0x00000000;
			private const int ABM_REMOVE = 0x00000001;
			private const int ABM_QUERYPOS = 0x00000002;
			private const int ABM_SETPOS = 0x00000003;
			private const int ABM_GETSTATE = 0x00000004;
			private const int ABM_GETTASKBARPOS = 0x00000005;
			private const int ABM_ACTIVATE = 0x00000006;  // lParam == TRUE/FALSE means activate/deactivate
			private const int ABM_GETAUTOHIDEBAR = 0x00000007;
			private const int ABM_SETAUTOHIDEBAR = 0x00000008;

			// Appbar edge constants
			private const int ABE_LEFT = 0;
			private const int ABE_TOP = 1;
			private const int ABE_RIGHT = 2;
			private const int ABE_BOTTOM = 3;

			// SystemParametersInfo constants
			private const System.UInt32 SPI_GETWORKAREA = 0x0030;

			public enum ScreenEdge
			{
				Undefined = -1,
				Left = ABE_LEFT,
				Top = ABE_TOP,
				Right = ABE_RIGHT,
				Bottom = ABE_BOTTOM
			}

			public ScreenEdge Edge
			{
				get	{ return (ScreenEdge)m_data.uEdge; }
			}

			public Rectangle WorkArea
			{
				get
				{
					Int32 bResult = 0;
					RECT rc = new RECT();
					IntPtr rawRect = System.Runtime.InteropServices.Marshal.AllocHGlobal(System.Runtime.InteropServices.Marshal.SizeOf(rc));
					bResult = SystemParametersInfo(SPI_GETWORKAREA, 0, rawRect, 0);
					rc = (RECT)System.Runtime.InteropServices.Marshal.PtrToStructure(rawRect, rc.GetType());

					if (bResult == 1)
					{
						System.Runtime.InteropServices.Marshal.FreeHGlobal(rawRect);
						return new Rectangle(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
					}

					return new Rectangle(0, 0, 0, 0);
				}
			}

			public void GetPosition(string strClassName, string strWindowName)
			{
				m_data = new APPBARDATA();
				m_data.cbSize = (UInt32)System.Runtime.InteropServices.Marshal.SizeOf(m_data.GetType());

				IntPtr hWnd = FindWindow(strClassName, strWindowName);

				if (hWnd != IntPtr.Zero)
				{
					UInt32 uResult = SHAppBarMessage(ABM_GETTASKBARPOS, ref m_data);

					if (uResult == 1)
					{
					}
					else
					{
						throw new Exception("Failed to communicate with the given AppBar");
					}
				}
				else
				{
					throw new Exception("Failed to find an AppBar that matched the given criteria");
				}
			}

			public void GetSystemTaskBarPosition()
			{
				GetPosition("Shell_TrayWnd", null);
			}
		}
		
		#endregion


    }
}
