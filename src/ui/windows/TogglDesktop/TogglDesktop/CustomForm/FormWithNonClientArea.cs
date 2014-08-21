#region Custom Border Forms - Copyright (C) 2005 Szymon Kobalczyk

// Custom Border Forms
// Copyright (C) 2005 Szymon Kobalczyk
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
// Szymon Kobalczyk (http://www.geekswithblogs.com/kobush)

#endregion

#region Using directives

using System;
using System.Collections;
using System.ComponentModel;
using System.Diagnostics;
using System.Drawing;
using System.Runtime.InteropServices;
using System.Windows.Forms;

using System.Reflection; // used for logging 

#endregion

namespace TogglDesktop
{
    // Nonclient Area
    // http://msdn.microsoft.com/library/default.asp?url=/library/en-us/gdi/pantdraw_9uht.asp
    // http://support.microsoft.com/kb/q99046/

    //see also http://www.google.pl/groups?q=ncpaint+group:*dotnet*&hl=pl&lr=&c2coff=1&selm=%23%245FnRUpEHA.2304%40TK2MSFTNGP14.phx.gbl&rnum=7
    //http://www.google.pl/groups?hl=pl&lr=&c2coff=1&selm=02EF5D19-D288-4C0E-818E-752DB176F5C0%40microsoft.com&rnum=15
    //http://www.codeguru.com/Cpp/controls/buttonctrl/advancedbuttons/article.php/c5179/

    //http://dotnetrix.co.uk/ncpanel.html

    //http://www.vbaccelerator.com/home/VB/Code/Libraries/XP_Visual_Styles/Drawing_with_XP_Visual_Styles/article.asp

    // Display a shaded title bar
    // http://www.vb-helper.com/howto_shaded_titlebar.html

    //http://www.catch22.net/tuts/titlebar.asp

    // The best of all
    //http://www.mindspring.com/~cityzoo/ttlbar1.html

    /// <summary>
    /// Extended form class that suports drawing in non-client area.
    /// </summary>
#if !DEBUGFORM
    [DebuggerStepThrough]
#endif
    [System.ComponentModel.DesignerCategory("code")]
    public class FormWithNonClientArea : Form
    {
        protected override CreateParams CreateParams
        {
            get
            {
                CreateParams cp = base.CreateParams;
                cp.Style = cp.Style | (int)NativeMethods.WindowStyle.WS_SYSMENU;
                return cp;
            }
        }

        private bool _nonClientAreaDoubleBuffering;

        [DefaultValue(false)]
        public virtual bool NonClientAreaDoubleBuffering
        {
            get { return _nonClientAreaDoubleBuffering; }
            set
            {
                _nonClientAreaDoubleBuffering = value;
                // No need to invalidate anything,
                // next painting will use double-buffering.
            }
        }

        private bool _enableNonClientAreaPaint = true;

        [DefaultValue(true)]
        public bool EnableNonClientAreaPaint
        {
            get { return _enableNonClientAreaPaint; }
            set
            {
                _enableNonClientAreaPaint = value;
                InvalidateWindow();
            }
        }

        protected override void OnHandleCreated(EventArgs e)
        {
            // Disable theming on current window so we don't get 
            // any funny artifacts (round corners, etc.)
            NativeMethods.SetWindowTheme(this.Handle, "", "");

#if !DEBUG
			// When application window stops responding to messages
			// system will finally loose patience and repaint it with default theme.
			// This prevents such behavior for entire application.
			// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winui/winui/windowsuserinterface/lowlevelclientsupport/misc/rtldisableprocesswindowsghosting.asp
			NativeMethods.DisableProcessWindowsGhosting();
#endif
            base.OnHandleCreated(e);
        }

        protected override void WndProc(ref Message m)
        {
            Log(MethodInfo.GetCurrentMethod(), "Message = {0}", (NativeMethods.WindowMessages)m.Msg);

            if (!this.EnableNonClientAreaPaint)
            {
                base.WndProc(ref m);
                return;
            }

			switch (m.Msg)
			{
				case (int)NativeMethods.WindowMessages.WM_NCCALCSIZE:
					{
						// Provides new coordinates for the window client area.
						WmNCCalcSize(ref m);
						break;
					}
				case (int)NativeMethods.WindowMessages.WM_NCHITTEST:
					{
						// Tell the system what element is at the current mouse location. 
						WmNCHitTest(ref m);
						break;
					}
				case (int)NativeMethods.WindowMessages.WM_NCPAINT:
					{
						// Here should all our painting occur, but...
						WmNCPaint(ref m);
						break;
					}
				case (int)NativeMethods.WindowMessages.WM_NCACTIVATE:
					{
						// ... WM_NCACTIVATE does some painting directly 
						// without bothering with WM_NCPAINT ...
						WmNCActivate(ref m);
						break;
					}
				case (int)NativeMethods.WindowMessages.WM_SETTEXT:
					{
						// ... and some painting is required in here as well
						WmSetText(ref m);
						break;
					}
				case (int)NativeMethods.WindowMessages.WM_NCMOUSEMOVE:
					{
						WmNCMouseMove(ref m);
						break;
					}
				case (int)NativeMethods.WindowMessages.WM_NCMOUSELEAVE:
					{
						WmNCMouseLeave(ref m);
						break;
					}
				case (int)NativeMethods.WindowMessages.WM_NCLBUTTONDOWN:
					{
						WmNCLButtonDown(ref m);
						break;
					}
				case 174: // ignore magic message number
					{
						Log(MethodInfo.GetCurrentMethod(), "### Magic message ### WParam = {0}", m.WParam.ToInt32());
						break;
					}
				case (int)NativeMethods.WindowMessages.WM_SYSCOMMAND:
					{
						WmSysCommand(ref m);
						break;
					}
                case (int)NativeMethods.WindowMessages.WM_WINDOWPOSCHANGED:
                    {
                        WmWindowPosChanged(ref m);
                        break;
                    }
                case (int)NativeMethods.WindowMessages.WM_ERASEBKGND:
                    {
                        WmEraseBkgnd(ref m);
                        break;
                    }
				default:
					{
						base.WndProc(ref m);
						break;
					}
			}
		}

        protected virtual void UpdateWindowState()
        {

        }

        private void WmEraseBkgnd(ref Message m)
        {
            base.WndProc(ref m);

            Log(MethodInfo.GetCurrentMethod(), "{0}", WindowState);
            UpdateWindowState();
        }

        private void WmWindowPosChanged(ref Message m)
        {
            base.WndProc(ref m);

            Log(MethodInfo.GetCurrentMethod(), "{0}", WindowState);
            UpdateWindowState();
        }

        protected virtual void OnSystemCommand(int sc)
        {
        }

		private void WmSysCommand(ref Message m)
		{
			Log(MethodInfo.GetCurrentMethod(), "{0}", (NativeMethods.SystemCommands)m.WParam.ToInt32());

			this.OnSystemCommand(m.WParam.ToInt32());

			DefWndProc(ref m);
		}

        public Point PointToWindow(Point screenPoint)
        {
            return new Point(screenPoint.X - Location.X, screenPoint.Y - Location.Y);
        }

        /// <summary>
        /// Adjust the supplied Rectangle to the desired position of the client rectangle. 
        /// </summary>
        protected virtual void OnNonClientAreaCalcSize(ref Rectangle bounds, bool update)
        { }

        private void WmNCCalcSize(ref Message m)
        {
            // http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winui/winui/windowsuserinterface/windowing/windows/windowreference/windowmessages/wm_nccalcsize.asp
            // http://groups.google.pl/groups?selm=OnRNaGfDEHA.1600%40tk2msftngp13.phx.gbl

            if (m.WParam == NativeMethods.FALSE)
            {
                NativeMethods.RECT ncRect = (NativeMethods.RECT)m.GetLParam(typeof(NativeMethods.RECT));
                Rectangle proposed = ncRect.Rect;

                Log(MethodInfo.GetCurrentMethod(), string.Format("### Client Rect [0] = ({0},{1}) x ({2},{3})",
                    proposed.Left, proposed.Top, proposed.Width, proposed.Height));

                OnNonClientAreaCalcSize(ref proposed, true);
                ncRect = NativeMethods.RECT.FromRectangle(proposed);

                Marshal.StructureToPtr(ncRect, m.LParam, false);
            }
            else if (m.WParam == NativeMethods.TRUE)
            {
                NativeMethods.NCCALCSIZE_PARAMS ncParams = (NativeMethods.NCCALCSIZE_PARAMS)m.GetLParam(typeof(NativeMethods.NCCALCSIZE_PARAMS));
                Rectangle proposed = ncParams.rectProposed.Rect;

                Log(MethodInfo.GetCurrentMethod(), string.Format("### Client Rect [1] = ({0},{1}) x ({2},{3})",
                    proposed.Left, proposed.Top, proposed.Width, proposed.Height));

                OnNonClientAreaCalcSize(ref proposed, true);
                ncParams.rectProposed = NativeMethods.RECT.FromRectangle(proposed);

                Marshal.StructureToPtr(ncParams, m.LParam, false);
            }
            m.Result = IntPtr.Zero;
        }

        //protected override void OnClientSizeChanged(EventArgs e)
        //{
        //    base.OnClientSizeChanged(e);

        //    Log(MethodInfo.GetCurrentMethod(), String.Format("$$$$$ ClientSize = ({0},{1}),  FormSize = ({2},{3})",
        //        ClientSize.Width, ClientSize.Height, Size.Width, Size.Height));
        //}

        protected override void SetClientSizeCore(int x, int y)
        {
            if (!EnableNonClientAreaPaint)
                base.SetClientSizeCore(x, y);

            this.Size = SizeFromClientSize(x, y, true);

            //  this.clientWidth = x;
            //  this.clientHeight = y;
            //  this.OnClientSizeChanged(EventArgs.Empty);
            // do this instead of above.
            this.UpdateBounds(Location.X, Location.Y, Size.Width, Size.Height, x, y);
        }

        private Size SizeFromClientSize(int x, int y, bool updatebuttons)
        {
            Rectangle bounds = new Rectangle(0, 0, 1000, 1000);
            OnNonClientAreaCalcSize(ref bounds, updatebuttons);

            return new Size(x + (1000 - bounds.Width), y + (1000 - bounds.Height));
        }

        protected override Size SizeFromClientSize(Size clientSize)
        {
            if (!EnableNonClientAreaPaint)
                return base.SizeFromClientSize(clientSize);

            return SizeFromClientSize(clientSize.Width, clientSize.Height, false);
        }

        /// <summary>
        /// Returns a value from a NCHITTEST enumeration specifing the window element on given point.
        /// </summary>
        protected virtual int OnNonClientAreaHitTest(Point clientPoint)
        {
            return (int)NativeMethods.NCHITTEST.HTCLIENT;
        }

        private void WmNCHitTest(ref Message m)
        {
            // http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winui/winui/windowsuserinterface/userinput/mouseinput/mouseinputreference/mouseinputmessages/wm_nchittest.asp

            Point screenPoint = new Point(m.LParam.ToInt32());
            Log(MethodInfo.GetCurrentMethod(), string.Format("### Screen Point ({0},{1})", screenPoint.X, screenPoint.Y));

            // convert to local coordinates
            Point clientPoint = PointToWindow(screenPoint);
            Log(MethodInfo.GetCurrentMethod(), string.Format("### Client Point ({0},{1})", clientPoint.X, clientPoint.Y));
            m.Result = new System.IntPtr(OnNonClientAreaHitTest(clientPoint));
        }

        /// <summary>
        /// Delivers new mouse position when it is moved over the non client area of the window. 
        /// </summary>
        protected virtual void OnNonClientMouseMove(MouseEventArgs args)
        {
        }

        private void WmNCMouseMove(ref Message msg)
        {
            // http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winui/winui/windowsuserinterface/userinput/mouseinput/mouseinputreference/mouseinputmessages/wm_nchittest.asp

            Point clientPoint = this.PointToWindow(new Point(msg.LParam.ToInt32()));
            OnNonClientMouseMove(new MouseEventArgs(MouseButtons.None, 0,
                clientPoint.X, clientPoint.Y, 0));
            msg.Result = IntPtr.Zero;
        }

        /// <summary>
        /// Called when mouse cursor leaves the non client window area.
        /// </summary>
        /// <param name="args"></param>
        protected virtual void OnNonClientMouseLeave(EventArgs args)
        {
        }

        private void WmNCMouseLeave(ref Message m)
        {
            // http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winui/winui/windowsuserinterface/userinput/mouseinput/mouseinputreference/mouseinputmessages/wm_ncmouseleave.asp
            OnNonClientMouseLeave(EventArgs.Empty);
        }

        /// <summary>
        /// Called each time one of the mouse buttons was pressed over the non-client area.
        /// </summary>
        /// <param name="args">NonClientMouseEventArgs contain mouse position, button pressed,
        /// and hit-test code for current position. </param>
        protected virtual void OnNonClientMouseDown(NonClientMouseEventArgs args)
        {
        }

        private void WmNCLButtonDown(ref Message msg)
        {
            Point pt = this.PointToWindow(new Point(msg.LParam.ToInt32()));
            NonClientMouseEventArgs args = new NonClientMouseEventArgs(
                MouseButtons.Left, 1, pt.X, pt.Y, 0, msg.WParam.ToInt32());
            OnNonClientMouseDown(args);
            if (!args.Handled)
            {
                DefWndProc(ref msg);
            }
            msg.Result = NativeMethods.TRUE;
        }

        private void WmNCPaint(ref Message msg)
        {
            // http://msdn.microsoft.com/library/default.asp?url=/library/en-us/gdi/pantdraw_8gdw.asp
            // example in q. 2.9 on http://www.syncfusion.com/FAQ/WindowsForms/FAQ_c41c.aspx#q1026q

            // The WParam contains handle to clipRegion or 1 if entire window should be repainted
            PaintNonClientArea(msg.HWnd, (IntPtr)msg.WParam);

            // we handled everything
            msg.Result = NativeMethods.TRUE;
        }

        private void WmSetText(ref Message msg)
        {
            // allow the system to receive the new window title
            DefWndProc(ref msg);

            // repaint title bar
            PaintNonClientArea(msg.HWnd, (IntPtr)1);
        }

        private void WmNCActivate(ref Message msg)
        {
            // http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winui/winui/windowsuserinterface/windowing/windows/windowreference/windowmessages/wm_ncactivate.asp

            bool active = (msg.WParam == NativeMethods.TRUE);
            msg.WParam = NativeMethods.TRUE;
            Log(MethodInfo.GetCurrentMethod(), "### Draw active title bar = {0}", active);

            if (WindowState == FormWindowState.Minimized)
                DefWndProc(ref msg);
            else
            {
                // repaint title bar
                PaintNonClientArea(msg.HWnd, (IntPtr)1);

                // allow to deactivate window
                msg.Result = NativeMethods.TRUE;
            }
        }

        /// <summary>
        /// Paints the client rect - e.ClipingRect has the correct window size, since this.Width, this.Height
        /// aren't always correct when calling this methode (because window is actually resizing)
        /// </summary>
        /// <param name="e"></param>
        protected virtual void OnNonClientAreaPaint(NonClientPaintEventArgs e)
        {
            e.Graphics.FillRectangle(new SolidBrush(Color.DarkViolet), 0, 0, e.Bounds.Width, e.Bounds.Height);
        }

        private void PaintNonClientArea(IntPtr hWnd, IntPtr hRgn)
        {
            NativeMethods.RECT windowRect = new NativeMethods.RECT();
            if (NativeMethods.GetWindowRect(hWnd, ref windowRect) == 0)
                return;

            Rectangle bounds = new Rectangle(0, 0,
                windowRect.right - windowRect.left,
                windowRect.bottom - windowRect.top);

            if (bounds.Width == 0 || bounds.Height == 0)
                return;

            // The update region is clipped to the window frame. When wParam is 1, the entire window frame needs to be updated. 
            Region clipRegion = null;
            if (hRgn != (IntPtr)1)
                clipRegion = System.Drawing.Region.FromHrgn(hRgn);

            // MSDN states that only WINDOW and INTERSECTRGN are needed,
            // but other sources confirm that CACHE is required on Win9x
            // and you need CLIPSIBLINGS to prevent painting on overlapping windows.
            IntPtr hDC = NativeMethods.GetDCEx(hWnd, hRgn,
                (int)(NativeMethods.DCX.DCX_WINDOW | NativeMethods.DCX.DCX_INTERSECTRGN
                    | NativeMethods.DCX.DCX_CACHE | NativeMethods.DCX.DCX_CLIPSIBLINGS));

            if (hDC == IntPtr.Zero)
                hDC = NativeMethods.GetWindowDC(hWnd);

            if (hDC == IntPtr.Zero)
                return;


            try
            {

                if (!this.NonClientAreaDoubleBuffering)
                {
                    using (Graphics g = Graphics.FromHdc(hDC))
                    {
                        //cliping rect is not cliping rect but actual rectangle
                        OnNonClientAreaPaint(new NonClientPaintEventArgs(g, bounds, clipRegion));
                    }

                    //NOTE: The Graphics object would realease the HDC on Dispose.
                    // So there is no need to call NativeMethods.ReleaseDC(msg.HWnd, hDC);
                    //http://groups.google.pl/groups?hl=pl&lr=&c2coff=1&client=firefox-a&rls=org.mozilla:en-US:official_s&threadm=%23DDSaH7BFHA.3644%40TK2MSFTNGP15.phx.gbl&rnum=15&prev=/groups%3Fq%3DWM_NCPaint%2B%2BGetDCEx%26start%3D10%26hl%3Dpl%26lr%3D%26c2coff%3D1%26client%3Dfirefox-a%26rls%3Dorg.mozilla:en-US:official_s%26selm%3D%2523DDSaH7BFHA.3644%2540TK2MSFTNGP15.phx.gbl%26rnum%3D15
                    //http://groups.google.pl/groups?hl=pl&lr=&c2coff=1&client=firefox-a&rls=org.mozilla:en-US:official_s&threadm=cmo00r%24j9v%241%40mamut1.aster.pl&rnum=1&prev=/groups%3Fq%3DDCX_PARENTCLIP%26hl%3Dpl%26lr%3D%26c2coff%3D1%26client%3Dfirefox-a%26rls%3Dorg.mozilla:en-US:official_s%26selm%3Dcmo00r%2524j9v%25241%2540mamut1.aster.pl%26rnum%3D1
                }
                else
                {
                    //http://www.codeproject.com/csharp/flicker_free.asp
                    //http://www.pinvoke.net/default.aspx/gdi32/BitBlt.html

                    IntPtr CompatiblehDC = NativeMethods.CreateCompatibleDC(hDC);
                    IntPtr CompatibleBitmap = NativeMethods.CreateCompatibleBitmap(hDC, bounds.Width, bounds.Height);

                    try
                    {
                        NativeMethods.SelectObject(CompatiblehDC, CompatibleBitmap);

                        // copy current screen to bitmap
                        //TODO: this is quite slow (80% of this method). Why?
                        NativeMethods.BitBlt(CompatiblehDC, 0, 0, bounds.Width, bounds.Height, hDC, 0, 0, NativeMethods.TernaryRasterOperations.SRCCOPY);

                        using (Graphics g = Graphics.FromHdc(CompatiblehDC))
                        {
                            OnNonClientAreaPaint(new NonClientPaintEventArgs(g, bounds, clipRegion));
                        }

                        // copy current from bitmap to screen
                        NativeMethods.BitBlt(hDC, 0, 0, bounds.Width, bounds.Height, CompatiblehDC, 0, 0, NativeMethods.TernaryRasterOperations.SRCCOPY);
                    }
                    finally
                    {
                        NativeMethods.DeleteObject(CompatibleBitmap);
                        NativeMethods.DeleteDC(CompatiblehDC);

                    }

#if !NET1X
                    // .NET 2.0 has this new class BufferedGraphics. But it paints the clien area in all black.
                    // I dont know how to use it properly.

                    //using (BufferedGraphics bg = BufferedGraphicsManager.Current.Allocate(hDC, bounds))
                    //{
                    //    Graphics g = bg.Graphics;
                    //    //Rectangle clientRect = this.ClientRectangle;
                    //    //clientRect.Offset(this.PointToScreen(Point.Empty));
                    //    //g2.SetClip(clientRect, CombineMode.Exclude);
                    //    OnNonClientAreaPaint(new NonClientPaintEventArgs(g, bounds, clipRegion));
                    //    bg.Render();
                    //}
#endif
                }
            }
            finally
            {
                NativeMethods.ReleaseDC(this.Handle, hDC);
            }
        }

        /// <summary>
        /// This method should invalidate entire window including the non-client area.
        /// </summary>
        protected void InvalidateWindow()
        {
            if (!IsDisposed && IsHandleCreated)
            {
                NativeMethods.SetWindowPos(this.Handle, IntPtr.Zero, 0, 0, 0, 0,
                    (int)(NativeMethods.SetWindowPosOptions.SWP_NOACTIVATE | NativeMethods.SetWindowPosOptions.SWP_NOMOVE | NativeMethods.SetWindowPosOptions.SWP_NOSIZE |
                    NativeMethods.SetWindowPosOptions.SWP_NOZORDER | NativeMethods.SetWindowPosOptions.SWP_NOOWNERZORDER | NativeMethods.SetWindowPosOptions.SWP_FRAMECHANGED));

                NativeMethods.RedrawWindow(this.Handle, IntPtr.Zero, IntPtr.Zero,
                    (int)(NativeMethods.RedrawWindowOptions.RDW_FRAME | NativeMethods.RedrawWindowOptions.RDW_UPDATENOW | NativeMethods.RedrawWindowOptions.RDW_INVALIDATE));
            }
        }

        #region Log
        [Conditional("DEBUGFORM")]
        protected internal static void Log(System.Reflection.MemberInfo callingMethod, string message, params object[] args)
        {
            if (args != null)
                message = String.Format(message, args);
            Debug.WriteLine(String.Format("{0}.{1} - {2}", callingMethod.DeclaringType.Name, callingMethod.Name, message));
        }
        #endregion //Log

    }

#if !DEBUGFORM
    [DebuggerStepThrough]
#endif
    public class NonClientMouseEventArgs : MouseEventArgs
    {
        private int _hitTest;
        private bool _handled;

        public NonClientMouseEventArgs(MouseButtons button, int clicks, int x, int y, int delta, int hitTest )
            : base(button, clicks, x, y, delta)
        {
            _hitTest = hitTest;
        }

        public int HitTest
        {
            get { return _hitTest; }
            set { _hitTest = value; }
        }

        public bool Handled
        {
            get { return _handled; }
            set { _handled = value; }
        }

    }

#if !DEBUGFORM
    [DebuggerStepThrough]
#endif
    public class NonClientPaintEventArgs : EventArgs
    {
        public NonClientPaintEventArgs(Graphics g, Rectangle bounds, Region clipRegion)
        {
            _graphics = g;
            _bounds = bounds;
            _clipRegion = clipRegion;
        }

        private Rectangle _bounds;
        public Rectangle Bounds
        {
            get { return _bounds; }
        }

        private Region _clipRegion;
        public Region ClipRegion
        {
            get { return _clipRegion; }
        }

        private Graphics _graphics;
        public Graphics Graphics
        {
            get { return _graphics; }
        }
    }
}
