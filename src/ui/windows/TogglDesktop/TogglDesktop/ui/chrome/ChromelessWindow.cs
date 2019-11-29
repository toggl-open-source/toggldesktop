using System;
using System.Drawing;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Forms;
using System.Windows.Interop;
using System.Windows.Media.Imaging;
using System.Windows.Shell;
using Application = System.Windows.Application;

namespace TogglDesktop
{
    public class ChromelessWindow : Window
    {
        private bool isToolWindow;

        private static IconBitmapDecoder iconActive;
        private static IconBitmapDecoder iconInactive;

        public virtual int WindowHeaderHeight { get { return 0; } }

        public ChromelessWindow()
        {
            this.WindowStartupLocation = WindowStartupLocation.CenterOwner;
            this.UseLayoutRounding = true;

            this.updateWindowChrome();

            this.SourceInitialized += this.onSourceInitialised;

            ensureIconsInitialised();
        }

        #region public properties

        public bool IsToolWindow
        {
            get { return this.isToolWindow; }
            set
            {
                if (this.IsInitialized)
                {
                    throw new InvalidOperationException("Can not change IsToolWindow after initialisation.");
                }

                this.isToolWindow = value;
            }
        }

        #endregion

        #region public methods

        public virtual void SetIconState(bool tracking)
        {
            this.Icon = (tracking ? iconActive : iconInactive).Frames[0];
        }

        public void Hide(bool activateOwner = true)
        {
            if (activateOwner)
            {
                var owner = this.Owner;
                if (owner != null)
                {
                    owner.Show();
                    if (owner.WindowState == WindowState.Minimized)
                        owner.WindowState = WindowState.Normal;
                    owner.Activate();
                }
            }

            base.Hide();
        }

        #endregion

        #region ui events

        protected override void OnInitialized(EventArgs e)
        {
            base.OnInitialized(e);

            if (this.isToolWindow)
            {
                this.turnIntoToolWindow();
            }
        }

        protected override void OnStateChanged(EventArgs e)
        {
            this.updateWindowChrome();

            base.OnStateChanged(e);
        }

        protected override void OnLocationChanged(EventArgs e)
        {
            if (!this.IsToolWindow)
            {
                this.updateMaximumSize();
                if (this.WindowState != WindowState.Maximized
                    && this.ResizeMode != ResizeMode.CanResize)
                {
                    this.ResizeMode = ResizeMode.CanResize;
                }
            }

            base.OnLocationChanged(e);
        }

        #endregion

        #region style controlling

        private void updateWindowChrome()
        {
            var chrome = new WindowChrome
            {
                CaptionHeight = this.WindowHeaderHeight,
                CornerRadius = new CornerRadius(0),
                GlassFrameThickness = new Thickness(1),
                UseAeroCaptionButtons = false
            };

            if (this.IsToolWindow)
            {
                chrome.ResizeBorderThickness = new Thickness(0);
            }

            if (this.WindowState == WindowState.Maximized)
            {
                chrome.ResizeBorderThickness = new Thickness(0);
            }

            WindowChrome.SetWindowChrome(this, chrome);
        }
        
        private void turnIntoToolWindow()
        {
            this.ResizeMode = ResizeMode.NoResize;
        }

        #endregion

        #region updating maximum size

        private void updateMaximumSize()
        {
            var screenRect = this.GetCurrentScreenRectangle();

            this.MaxWidth = screenRect.Width;
            this.MaxHeight = screenRect.Height;
        }

        #endregion

        #region icon loading

        private static void ensureIconsInitialised()
        {
            if (iconActive != null)
            {
                return;
            }

            iconActive = loadIconFromResource("toggl");
            iconInactive = loadIconFromResource("toggl_inactive");
        }

        private static IconBitmapDecoder loadIconFromResource(string iconName)
        {
            var stream = Application.GetResourceStream(
                new Uri("pack://application:,,,/TogglDesktop;component/Resources/" + iconName + ".ico"));

            if (stream == null)
                throw new Exception("Icon resource not found");

            return new IconBitmapDecoder(
                stream.Stream,
                BitmapCreateOptions.None,
                BitmapCacheOption.Default);
        }

        #endregion

        #region maximised size fix

        // taken from: http://stackoverflow.com/a/25392397/1175259

        void onSourceInitialised(object sender, System.EventArgs e)
        {
            var handle = new WindowInteropHelper(this).Handle;
            var handleSource = HwndSource.FromHwnd(handle);
            if (handleSource == null)
                return;
            handleSource.AddHook(this.windowProc);
        }

        private IntPtr windowProc(IntPtr hwnd, int msg, IntPtr wParam, IntPtr lParam, ref bool handled)
        {
            switch (msg)
            {
                case 0x0024:/* WM_GETMINMAXINFO */
                    this.wmGetMinMaxInfo(hwnd, lParam);
                    handled = true;
                    break;
            }

            return IntPtr.Zero;
        }

        private void wmGetMinMaxInfo(IntPtr hwnd, IntPtr lParam)
        {
            var mmi = (MinMaxInfo)Marshal.PtrToStructure(lParam, typeof(MinMaxInfo));

            var yOffset = 0;
            var xOffset = (int)SystemParameters.BorderWidth;

            var currentScreen = Screen.FromHandle(hwnd);
            var workArea = currentScreen.WorkingArea;
            var monitorArea = currentScreen.Bounds;
            mmi.ptMaxPosition.x = Math.Abs(workArea.Left - monitorArea.Left) + xOffset;
            mmi.ptMaxPosition.y = Math.Abs(workArea.Top - monitorArea.Top) + yOffset;
            mmi.ptMaxSize.x = Math.Abs(workArea.Right - workArea.Left) - xOffset;
            mmi.ptMaxSize.y = Math.Abs(workArea.Bottom - workArea.Top) - yOffset;

            var minSize = new System.Windows.Point(this.MinWidth, this.MinHeight);
            minSize = this.transformToDevice(minSize);

            mmi.ptMinTrackSize.x = (int)minSize.X;
            mmi.ptMinTrackSize.y = (int)minSize.Y;

            Marshal.StructureToPtr(mmi, lParam, true);
        }

        private System.Windows.Point transformToDevice(System.Windows.Point p)
        {
            var presentationSource = PresentationSource.FromVisual(this);
            if (presentationSource != null)
            {
                var compositionTarget = presentationSource.CompositionTarget;
                if (compositionTarget != null)
                {
                    var t = compositionTarget.TransformToDevice;

                    return t.Transform(p);
                }
            }

            return p;
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct MinMaxInfo
        {
            // ReSharper disable FieldCanBeMadeReadOnly.Local
            // ReSharper disable MemberCanBePrivate.Local
            public Point ptReserved;
            public Point ptMaxSize;
            public Point ptMaxPosition;
            public Point ptMinTrackSize;
            public Point ptMaxTrackSize;
            // ReSharper restore MemberCanBePrivate.Local
            // ReSharper restore FieldCanBeMadeReadOnly.Local
        };

        [StructLayout(LayoutKind.Sequential)]
        private struct Point
        {
            public int x;
            public int y;
        }

        #endregion
    }
}