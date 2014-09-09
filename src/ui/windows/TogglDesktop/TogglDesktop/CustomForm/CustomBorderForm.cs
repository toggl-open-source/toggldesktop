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
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Windows.Forms;

using System.Reflection;
using System.Runtime.InteropServices;
using System.Diagnostics;
using System.Drawing.Imaging; // used for logging 

#endregion

namespace TogglDesktop
{

#if !DEBUGFORM
	[DebuggerStepThrough]
#else
    [System.ComponentModel.DesignerCategory("code")]
#endif
    public class CustomBorderForm : FormWithNonClientArea
	{

		private CaptionButton closeButton;
		private CaptionButton maximizeButton;
		private CaptionButton minimizeButton;
		private CaptionButton helpButton;

        private CaptionButtonCollection _captionButtons = new CaptionButtonCollection();

        private CaptionButtonCollection CaptionButtons
        {
            get { return _captionButtons; }
        }

		public CustomBorderForm()
		{
			closeButton = new CaptionButton();
			closeButton.Key = "close";
			closeButton.Visible = true;
			closeButton.HitTestCode = (int)NativeMethods.NCHITTEST.HTCLOSE;
			_captionButtons.Add(closeButton);

			maximizeButton = new CaptionButton();
			maximizeButton.Key = "maximize";
            maximizeButton.Enabled = this.MaximizeBox;
			maximizeButton.HitTestCode = (int)NativeMethods.NCHITTEST.HTMAXBUTTON;
			_captionButtons.Add(maximizeButton);

			minimizeButton = new CaptionButton();
			minimizeButton.Key = "minimize";
			minimizeButton.Enabled = this.MinimizeBox;
			minimizeButton.HitTestCode = (int)NativeMethods.NCHITTEST.HTMINBUTTON;
			_captionButtons.Add(minimizeButton);

			this.minimizeButton.Visible = this.maximizeButton.Visible
				= this.maximizeButton.Enabled | this.minimizeButton.Enabled;

			helpButton = new CaptionButton();
			helpButton.Key = "help";
			helpButton.Visible = this.HelpButton;
			helpButton.HitTestCode = (int)NativeMethods.NCHITTEST.HTHELP;
			_captionButtons.Add(helpButton);

            UpdateActiveFormStyle();
            FormStyleManager.StyleChanged += new EventHandler(FormStyleManager_StyleChanged);
		}

        void FormStyleManager_StyleChanged(object sender, EventArgs args)
        {
            UpdateActiveFormStyle();
        }

        private FormStyle _formStyle;

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        public FormStyle FormStyle
        {
            get { return _formStyle; }
            set 
            {
                if (_formStyle != value)
                {
                    _formStyle = value;
                    UpdateActiveFormStyle();
                }
            }
        }


        private bool _useFormStyleManager;

        [Category("Appearance")]
        [DefaultValue(false)]
        public bool UseFormStyleManager
        {
            get { return _useFormStyleManager; }
            set {
                if (_useFormStyleManager != value)
                {
                    _useFormStyleManager = value;
                    UpdateActiveFormStyle();
                }
            }
        }

        private string _formStyleName;

        [Category("Appearance")]
        [DefaultValue(null)]
        public string FormStyleName
        {
            get { return _formStyleName; }
            set {
                if (_formStyleName != value)
                {
                    _formStyleName = value;
                    UpdateActiveFormStyle();
                }
            }
        }

        private void UpdateActiveFormStyle()
        {
            FormStyle style = null;
            if (UseFormStyleManager)
            {
                // try to load specified style
                if (!String.IsNullOrEmpty(FormStyleName))
                    style = FormStyleManager.GetStyle(FormStyleName);

                // if it wasn't found try to load default style
                if (style == null)
                    style = FormStyleManager.GetDefaultStyle();
            }
            else
            {
                style = FormStyle;
            }
            ActiveFormStyle = style;
        }

        private FormStyle _activeformStyle;
        
        protected FormStyle ActiveFormStyle
		{
			get { return _activeformStyle; }
            set
            {
                if (_activeformStyle != value)
                {
                    if (_activeformStyle != null)
                        _activeformStyle.ChildPropertyChanged -= new ChildPropertyChangedEventHandler(FormStyle_ChildPropertyChanged);

                    _activeformStyle = value;

                    if (_activeformStyle != null)
                        _activeformStyle.ChildPropertyChanged += new ChildPropertyChangedEventHandler(FormStyle_ChildPropertyChanged);

                    UpdateButtonAppearance();
                    InvalidateWindow();
                }
            }
		}

        void FormStyle_ChildPropertyChanged(object sender, ChildPropertyChangedEventArgs args)
        {
            InvalidateWindow();
        }

        private void UpdateButtonAppearance()
        {
            if (ActiveFormStyle != null)
            {
                closeButton.Appearance = ActiveFormStyle.CloseButton;
                minimizeButton.Appearance = ActiveFormStyle.MinimizeButton;
                maximizeButton.Appearance = (this.WindowState == FormWindowState.Maximized) ?
                    ActiveFormStyle.RestoreButton : ActiveFormStyle.MaximizeButton;
                helpButton.Appearance = ActiveFormStyle.HelpButton;
            }
            else
            {
                closeButton.Appearance = null;
                minimizeButton.Appearance = null;
                maximizeButton.Appearance = null;
                helpButton.Appearance = null;
            }
        }


		public new bool MaximizeBox
		{
			get { return base.MaximizeBox; }
			set {
				this.maximizeButton.Enabled = value;
				this.minimizeButton.Visible = this.maximizeButton.Visible 
					= this.maximizeButton.Enabled | this.minimizeButton.Enabled;
				base.MaximizeBox = value; 
			}
		}

		public new bool MinimizeBox
		{
			get { return base.MinimizeBox; }
			set
			{
				this.minimizeButton.Enabled = value;
				this.minimizeButton.Visible = this.maximizeButton.Visible
					= this.maximizeButton.Enabled | this.minimizeButton.Enabled;
				base.MinimizeBox = value;
			}
		}

		public new bool ControlBox
		{
			get { return base.ControlBox; }
			set
			{
				this.closeButton.Enabled = value;
				base.ControlBox = value;
				
			}
		}

		public new bool HelpButton
		{
			get { return base.HelpButton; }
			set
			{
				this.helpButton.Visible = value;
				base.HelpButton = value;
			}
		}

        protected override void OnNonClientAreaCalcSize(ref Rectangle bounds, bool update)
		{
			if (ActiveFormStyle == null)
				return;

            if (update)
			    UpdateCaptionButtonBounds(bounds);

            Padding clientPadding = ActiveFormStyle.ClientAreaPadding;
            bounds = new Rectangle(bounds.Left + clientPadding.Left, bounds.Top + clientPadding.Top,
                bounds.Width - clientPadding.Horizontal, bounds.Height - clientPadding.Vertical);
		}

		private void UpdateCaptionButtonBounds(Rectangle windowRect)
		{
            // start from top-right corner
			int x = windowRect.Width;
				
			foreach (CaptionButton button in this.CaptionButtons)
			{
				if (button.Visible)
				{
                    int y = button.Appearance.Margin.Top;
                    x -= (button.Appearance.Size.Width + button.Appearance.Margin.Right);
					button.Bounds = new Rectangle(x, y,
                        button.Appearance.Size.Width, button.Appearance.Size.Height);
					x -= button.Appearance.Margin.Left;
				}

                // background will be recreated on next redraw
                button.BackgroundImage = null;
			}

			// Should I move this where this actually changes WM_GETMINMAXINFO ??
			//maximizeButton.Appearance = (this.WindowState == FormWindowState.Maximized) ?
			//    _borderAppearance.RestoreButton : _borderAppearance.MaximizeButton;
		}

		protected override void OnNonClientMouseLeave(EventArgs args)
		{
			if (!trakingMouse)
				return;

			foreach(CaptionButton button in this.CaptionButtons)
			{
				if (button.State != CaptionButtonState.Normal)
				{
					button.State = CaptionButtonState.Normal;
					DrawButton(button);
					UnhookMouseEvent();
				}
			}
		}

		protected override void OnNonClientMouseDown(NonClientMouseEventArgs args)
		{
			if (args.Button != MouseButtons.Left)
				return;

			switch (args.HitTest)
			{
				case (int)NativeMethods.NCHITTEST.HTCLOSE:
					{
						//NativeMethods.POINT pt = NativeMethods.POINT.FromPoint(args.Location);
						if (DepressButton(closeButton))
							NativeMethods.SendMessage(this.Handle,
								(int)NativeMethods.WindowMessages.WM_SYSCOMMAND,
								(IntPtr)NativeMethods.SystemCommands.SC_CLOSE, IntPtr.Zero);
						args.Handled = true;
						break;
					}
				case (int)NativeMethods.NCHITTEST.HTMINBUTTON:
					{
				//		NativeMethods.POINT pt = NativeMethods.POINT.FromPoint(args.Location);
						if (DepressButton(minimizeButton))
							NativeMethods.SendMessage(this.Handle,
								(int)NativeMethods.WindowMessages.WM_SYSCOMMAND,
								(IntPtr)NativeMethods.SystemCommands.SC_MINIMIZE, IntPtr.Zero);
						args.Handled = true;
						break;
					}
				case (int)NativeMethods.NCHITTEST.HTMAXBUTTON:
					{
						if (DepressButton(maximizeButton)) 
						{
							int sc = (WindowState == FormWindowState.Maximized)?
								(int)NativeMethods.SystemCommands.SC_RESTORE : (int)NativeMethods.SystemCommands.SC_MAXIMIZE;

							NativeMethods.SendMessage(this.Handle,
								(int)NativeMethods.WindowMessages.WM_SYSCOMMAND,
								(IntPtr)sc, IntPtr.Zero);
						}
						args.Handled = true;
						break;
					}
                case (int)NativeMethods.NCHITTEST.HTHELP:
                    {
                        if (DepressButton(helpButton))
                        {
                            Type type = this.GetType();
                            if (type != null)
                            {
                                MethodInfo methodInfo = type.GetMethod("toggleMenu");
                                methodInfo.Invoke(this, new object[] {});
                            }
                        }
                        args.Handled = true;
                        break;
                    }
			}
			//TODO: handle other buttons if exist
		}

        protected override void UpdateWindowState()
        {
            if (ActiveFormStyle == null)
                return; 

            FormButtonStyle newAppearance;
            if (this.WindowState == FormWindowState.Maximized)
                newAppearance = this.ActiveFormStyle.RestoreButton;
            else
                newAppearance = this.ActiveFormStyle.MaximizeButton;

            if (newAppearance != maximizeButton.Appearance)
            {
                maximizeButton.Appearance = newAppearance;
                DrawButton(maximizeButton);
            }
        }

		private void DrawButton(CaptionButton button)
		{
            if (IsHandleCreated)
            {
                // MSDN states that only WINDOW and INTERSECTRGN are needed,
                // but other sources confirm that CACHE is required on Win9x
                // and you need CLIPSIBLINGS to prevent painting on overlapping windows.
                IntPtr hDC = NativeMethods.GetDCEx(this.Handle, (IntPtr)1,
                    (int)(NativeMethods.DCX.DCX_WINDOW | NativeMethods.DCX.DCX_INTERSECTRGN
                        | NativeMethods.DCX.DCX_CACHE | NativeMethods.DCX.DCX_CLIPSIBLINGS));

			    if (hDC != IntPtr.Zero)
			    {
				    using (Graphics g = Graphics.FromHdc(hDC))
				    {
					    button.DrawButton(g, true);
				    }
			    }

                NativeMethods.ReleaseDC(this.Handle, hDC);
            }
		}

		private bool DepressButton(CaptionButton currentButton)
		{
			bool result = false;
			try
			{
				// callect all mouse events until WL_BUTTONUP
				this.Capture = true;		// hopefully doeas the same as SetCapture(handle)
				
				// draw button in pressed mode
				currentButton.State = CaptionButtonState.Pressed;
				DrawButton(currentButton);

				// loop untill button is released
				bool done = false;
				while (!done)
				{
					// NOTE: This struct needs to be here. We had strange error (starting from Beta 2).
					// when this was defined at begining of this method. also check if types are correct (no overlap). 
					Message m = new Message();
					if (NativeMethods.PeekMessage(ref m, IntPtr.Zero,
						(int)NativeMethods.WindowMessages.WM_MOUSEFIRST, (int)NativeMethods.WindowMessages.WM_MOUSELAST,
						(int)NativeMethods.PeekMessageOptions.PM_REMOVE))
					{
						Log(MethodInfo.GetCurrentMethod(), "Message = {0}, Button = {1}", (NativeMethods.WindowMessages)m.Msg, currentButton);
						switch(m.Msg)
						{
						    case (int)NativeMethods.WindowMessages.WM_LBUTTONUP:
								{
									if (currentButton.State == CaptionButtonState.Pressed)
									{
										currentButton.State = CaptionButtonState.Normal;
										DrawButton(currentButton);
									}
									Point pt = PointToWindow(PointToScreen(new Point(m.LParam.ToInt32())));
									Log(MethodInfo.GetCurrentMethod(), "### Point = ({0}, {1})", pt.X, pt.Y);
									result = currentButton.Bounds.Contains(pt);
									done = true;
									break;
								}
							case (int)NativeMethods.WindowMessages.WM_MOUSEMOVE:
								{
									Point clientPoint = PointToWindow(PointToScreen(new Point(m.LParam.ToInt32())));
									if (currentButton.Bounds.Contains(clientPoint))
									{
										if (currentButton.State == CaptionButtonState.Normal)
										{
											currentButton.State = CaptionButtonState.Pressed;
											DrawButton(currentButton);
										}
									}
									else
									{
										if (currentButton.State == CaptionButtonState.Pressed)
										{
											currentButton.State = CaptionButtonState.Normal;
											DrawButton(currentButton);
										}
									}
									break;
								}
						}
					}
				}
			}
			finally 
			{
				this.Capture = false;
			}

			return result;
		}

		protected override int OnNonClientAreaHitTest(Point p)
		{
            if (ActiveFormStyle == null)
                return (int)NativeMethods.NCHITTEST.HTCLIENT;

			foreach (CaptionButton button in this.CaptionButtons)
			{
				if (button.Visible && button.Bounds.Contains(p) && button.HitTestCode > 0)
					return button.HitTestCode;
			}

            if (FormBorderStyle != FormBorderStyle.FixedToolWindow &&
                FormBorderStyle != FormBorderStyle.SizableToolWindow)
            {
                if (GetIconRectangle().Contains(p))
                    return (int)NativeMethods.NCHITTEST.HTSYSMENU;
            }

			if (this.FormBorderStyle == FormBorderStyle.Sizable
				|| this.FormBorderStyle == FormBorderStyle.SizableToolWindow)
			{
				#region Handle sizable window borders
				if (p.X <= ActiveFormStyle.SizingBorderWidth) // left border
				{
					if (p.Y <= ActiveFormStyle.SizingCornerOffset)
						return (int)NativeMethods.NCHITTEST.HTTOPLEFT;
					else if (p.Y >= this.Height - ActiveFormStyle.SizingCornerOffset)
						return (int)NativeMethods.NCHITTEST.HTBOTTOMLEFT;
					else
						return (int)NativeMethods.NCHITTEST.HTLEFT;
				}
				else if (p.X >= this.Width - ActiveFormStyle.SizingBorderWidth) // right border
				{
					if (p.Y <= ActiveFormStyle.SizingCornerOffset)
						return (int)NativeMethods.NCHITTEST.HTTOPRIGHT;
					else if (p.Y >= this.Height - ActiveFormStyle.SizingCornerOffset)
						return (int)NativeMethods.NCHITTEST.HTBOTTOMRIGHT;
					else
						return (int)NativeMethods.NCHITTEST.HTRIGHT;
				}
				else if (p.Y <= ActiveFormStyle.SizingBorderWidth) // top border
				{
					if (p.X <= ActiveFormStyle.SizingCornerOffset)
						return (int)NativeMethods.NCHITTEST.HTTOPLEFT;
					if (p.X >= this.Width - ActiveFormStyle.SizingCornerOffset)
						return (int)NativeMethods.NCHITTEST.HTTOPRIGHT;
					else
						return (int)NativeMethods.NCHITTEST.HTTOP;
				}
				else if (p.Y >= this.Height - ActiveFormStyle.SizingBorderWidth) // bottom border
				{
					if (p.X <= ActiveFormStyle.SizingCornerOffset)
						return (int)NativeMethods.NCHITTEST.HTBOTTOMLEFT;
					if (p.X >= this.Width - ActiveFormStyle.SizingCornerOffset)
						return (int)NativeMethods.NCHITTEST.HTBOTTOMRIGHT;
					else
						return (int)NativeMethods.NCHITTEST.HTBOTTOM;
				}
				#endregion
			}

            // title bar
			if (p.Y <= ActiveFormStyle.ClientAreaPadding.Top)
				return (int)NativeMethods.NCHITTEST.HTCAPTION;

            // rest of non client area
			if (p.X <= this.ActiveFormStyle.ClientAreaPadding.Left || p.X >= this.ActiveFormStyle.ClientAreaPadding.Right
				|| p.Y >= this.ActiveFormStyle.ClientAreaPadding.Bottom)
				return (int)NativeMethods.NCHITTEST.HTBORDER;

			return (int)NativeMethods.NCHITTEST.HTCLIENT;
		}

		protected override void OnNonClientMouseMove(MouseEventArgs mouseEventArgs)
		{
			foreach (CaptionButton button in this.CaptionButtons)
			{
				if (button.Visible && button.Bounds.Contains(mouseEventArgs.X, mouseEventArgs.Y) && button.HitTestCode > 0)
				{
					if (button.State != CaptionButtonState.Over)
					{
						button.State = CaptionButtonState.Over;
						DrawButton(button);
						HookMouseEvent();
					}
				}
				else
				{
					if (button.State != CaptionButtonState.Normal)
					{
						button.State = CaptionButtonState.Normal;
						DrawButton(button);
						UnhookMouseEvent();
					}
				}
			}
		}

		NativeMethods.TRACKMOUSEEVENT trackMouseEvent;
		bool trakingMouse = false;

		private void HookMouseEvent()
		{
			if (!trakingMouse)
			{
				trakingMouse = true;
				if (this.trackMouseEvent == null)
				{
					this.trackMouseEvent = new NativeMethods.TRACKMOUSEEVENT();
					this.trackMouseEvent.dwFlags =
						(int)(NativeMethods.TrackMouseEventFalgs.TME_HOVER |
							  NativeMethods.TrackMouseEventFalgs.TME_LEAVE | 
							  NativeMethods.TrackMouseEventFalgs.TME_NONCLIENT);

					this.trackMouseEvent.hwndTrack = this.Handle;
				}

				if (NativeMethods.TrackMouseEvent(this.trackMouseEvent) == false)
					// use getlasterror to see whats wrong
					Log(MethodInfo.GetCurrentMethod(), "Failed enabling TrackMouseEvent: error {0}", 
						NativeMethods.GetLastError());
			}
		}

		private void UnhookMouseEvent()
		{
			trakingMouse = false;
		}

        private Rectangle GetIconRectangle()
        {
            return new Rectangle(ActiveFormStyle.IconPadding.Left, ActiveFormStyle.IconPadding.Top, 16, 16);
        }

        Icon smallIcon;

        protected override void Dispose(bool disposing)
        {
            base.Dispose(disposing);
            if (disposing) 
            {
                if (smallIcon != null)
                    smallIcon.Dispose();

                FormStyleManager.StyleChanged -= new EventHandler(FormStyleManager_StyleChanged);
                ActiveFormStyle = null;
            }
        }

        public new Icon Icon
        {
            get { return base.Icon; }
            set
            {
                if (value != Icon)
                {
                    if (this.smallIcon != null)
                    {
                        smallIcon.Dispose();
                        smallIcon = null;
                    }
                    try
                    {
                        smallIcon = new Icon(value, SystemInformation.SmallIconSize);
                    }
                    catch
                    { }
                }
                base.Icon = value;
            }
        }

        protected override void OnPaintBackground(PaintEventArgs e)
        {
            base.OnPaintBackground(e);

            if (ActiveFormStyle == null || ActiveFormStyle.NormalState.Image == null)
                return;

            Rectangle srcRect =
                DrawUtil.ExcludePadding(new Rectangle(Point.Empty, ActiveFormStyle.NormalState.Image.Size), 
                    ActiveFormStyle.ClientAreaPadding);

            Padding margins = 
                DrawUtil.SubstractPadding(ActiveFormStyle.NormalState.StretchMargins, ActiveFormStyle.ClientAreaPadding);

            DrawUtil.DrawImage(e.Graphics, ActiveFormStyle.NormalState.Image, srcRect, ClientRectangle, null, margins);
        }

        protected override void OnNonClientAreaPaint(NonClientPaintEventArgs e)
        {
			if (ActiveFormStyle == null)
				return;

            // assign clip region to exclude client area
            Region clipRegion = new Region(e.Bounds);
            clipRegion.Exclude(DrawUtil.ExcludePadding(e.Bounds, ActiveFormStyle.ClientAreaPadding));
            e.Graphics.Clip = clipRegion;

            // paint borders
            ActiveFormStyle.NormalState.DrawImage(e.Graphics, e.Bounds);

            int textOffset = 0;

            // paint icon
			if (ShowIcon &&
                FormBorderStyle != FormBorderStyle.FixedToolWindow &&
				FormBorderStyle != FormBorderStyle.SizableToolWindow)
			{
                Rectangle iconRect = GetIconRectangle();
                textOffset += iconRect.Right;

                if (smallIcon != null)
                    e.Graphics.DrawIconUnstretched(smallIcon, iconRect);
                else
                    e.Graphics.DrawIcon(Icon, iconRect);
			}


            // paint caption
            string text = this.Text;
            if (!String.IsNullOrEmpty(text))
			{
				// disable text wrapping and request elipsis characters on overflow
                using (StringFormat sf = new StringFormat())
                {
                    sf.Trimming = StringTrimming.EllipsisCharacter;
                    sf.FormatFlags = StringFormatFlags.NoWrap;
                    sf.LineAlignment = StringAlignment.Center;

                    // find position of the first button from left
                    int firstButton = e.Bounds.Width;
                    foreach (CaptionButton button in this.CaptionButtons)
                        if (button.Visible)
                            firstButton = Math.Min(firstButton, button.Bounds.X);

                    Padding padding = ActiveFormStyle.TitlePadding;
                    Rectangle textRect = new Rectangle(textOffset + padding.Left,
                        padding.Top, firstButton - textOffset - padding.Horizontal,
                        ActiveFormStyle.ClientAreaPadding.Top - padding.Vertical);

                    Font textFont = this.Font;
                    if (ActiveFormStyle.TitleFont != null)
                        textFont = ActiveFormStyle.TitleFont;

                    if (!ActiveFormStyle.TitleShadowColor.IsEmpty)
                    {
                        Rectangle shadowRect = textRect;
                        shadowRect.Offset(1, 1);

                        // draw drop shadow
                        using (Brush b = new SolidBrush(ActiveFormStyle.TitleShadowColor))
                        {
                            e.Graphics.DrawString(text, textFont, b, shadowRect, sf);
                        }
                    }

                    if (!ActiveFormStyle.TitleColor.IsEmpty)
                    {
                        // draw text
                        using (Brush b = new SolidBrush(ActiveFormStyle.TitleColor))
                        {
                            e.Graphics.DrawString(text, textFont, b, textRect, sf);
                        }
                    }
                }
			}

			// paint buttons
			foreach (CaptionButton button in this.CaptionButtons)
				button.DrawButton(e.Graphics, false);
		}

		public enum CaptionButtonState
		{
			Normal, Pressed, Over
		}

		#region class CaptionButton

#if !DEBUGFORM
	[DebuggerStepThrough]
#endif
		private class CaptionButton
		{
			private Rectangle _bounds;
			private CaptionButtonState _state;
			private FormButtonStyle _appearance;
			private string _key;
			private bool _visible = true;
			private int _hitTestCode = -1;
			private bool _enabled = true;

			public override string ToString()
			{
				return this.Key;
			}

			public CaptionButtonState State
			{
				get { return _state; }
				set { _state = value; }
			}

			public Rectangle Bounds
			{
				get { return _bounds; }
				set { _bounds = value; }
			}

			public FormButtonStyle Appearance
			{
				get { return _appearance; }
				set { _appearance = value; }
			}

			public string Key
			{
				get { return _key; }
				set { _key = value; }
			}

			public int HitTestCode
			{
				get { return _hitTestCode; }
				set { _hitTestCode = value; }
			}

			public bool Visible
			{
				get { return _visible; }
				set { _visible = value; }
			}

			public bool Enabled
			{
				get { return _enabled; }
				set { _enabled = value; }
			}

            private Image _backgroundImage;

            public Image BackgroundImage
            {
                get { return _backgroundImage; }
                set { _backgroundImage = value; }
            }

			public void DrawButton(Graphics g, bool paintBackground)
			{
				if (!Visible)
					return;

                if (paintBackground && BackgroundImage != null)
                    g.DrawImage(BackgroundImage, Bounds);

				if (this.Enabled)
				{
					switch (this.State)
					{
						case CaptionButtonState.Normal:
                            if (Appearance.NormalState != null)
                                Appearance.NormalState.DrawImage(g, Bounds);
							break;
						case CaptionButtonState.Pressed:
							if (Appearance.ActiveState != null)
                                Appearance.ActiveState.DrawImage(g, Bounds);
							break;
						case CaptionButtonState.Over:
							if (Appearance.HoverState != null)
                                Appearance.HoverState.DrawImage(g, Bounds);
							break;
					}
				}
				else
				{
                    Appearance.DisabledState.DrawImage(g, Bounds);
				}
			}
		}

		#endregion // END CaptionButton

		#region class CaptionButtonCollection
#if !DEBUGFORM
	[DebuggerStepThrough]
#endif
		private class CaptionButtonCollection : CollectionBase
		{
			public void Add(CaptionButton button)
			{
				this.List.Add(button);
			}
		}

		#endregion

	}	// END CustomBorderForm



}
