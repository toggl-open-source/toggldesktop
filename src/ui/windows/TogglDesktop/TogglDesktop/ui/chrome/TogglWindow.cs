using System;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Forms.VisualStyles;
using System.Windows.Interop;
using System.Windows.Media.Imaging;
using System.Windows.Shell;
using Screen = System.Windows.Forms.Screen;
using Rectangle = System.Drawing.Rectangle;

namespace TogglDesktop
{
    public class TogglWindow : Window
    {
        private static IconBitmapDecoder iconActive;
        private static IconBitmapDecoder iconInactive;

        private bool isToolWindow;
        private bool canClickIcon;

        private TogglChrome chrome;

        public int WindowHeaderHeight { get { return 40; } }

        public TogglWindow()
        {
            this.WindowStartupLocation = WindowStartupLocation.CenterOwner;
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

        public TogglChrome Chrome
        {
            get { return this.chrome; }
        }

        public bool CanClickIcon
        {
            get { return this.canClickIcon; }
            set
            {
                if (this.IsInitialized)
                {
                    throw new InvalidOperationException("Can not change IsToolWindow after initialisation.");
                }

                this.canClickIcon = value;
            }
        }

        #endregion

        #region public methods

        public void SetIconState(bool tracking)
        {
            this.Icon = (tracking ? iconActive : iconInactive).Frames[0];

            this.chrome.SetIconState(tracking);
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

        #region chrome initialisation

        protected override void OnInitialized(EventArgs e)
        {
            base.OnInitialized(e);

            if (this.IsToolWindow)
            {
                this.turnIntoToolWindow();
            }

            this.replaceChrome();
        }

        private void replaceChrome()
        {
            this.chrome = new TogglChrome
            {
                DataContext = this,
                IsToolWindow = this.IsToolWindow,
                CanClickIcon = this.CanClickIcon
            };

            this.SetIconState(true);

            this.setupChromeEvents();

            var oldContent = this.Content;
            this.Content = this.chrome;

            this.chrome.SetContent(oldContent as UIElement);
        }

        private void turnIntoToolWindow()
        {
            this.ResizeMode = ResizeMode.NoResize;

            this.isToolWindow = true;
        }

        private void setupChromeEvents()
        {
            this.chrome.IconButton.Click += this.onIconButtonClick;
            this.chrome.CloseButton.Click += this.onCloseButtonClick;
            this.chrome.MinimizeButton.Click += this.onMinimizeButtonClick;
            this.chrome.CogButton.Click += this.onCogButtonClick;
        }

        #endregion

        #region ui events

        protected virtual void onIconButtonClick(object sender, RoutedEventArgs e)
        {
            
        }

        protected virtual void onCogButtonClick(object sender, RoutedEventArgs e)
        {
        }

        protected virtual void onMinimizeButtonClick(object sender, RoutedEventArgs e)
        {
            this.WindowState = WindowState.Minimized;
        }

        protected virtual void onCloseButtonClick(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        protected override void OnLocationChanged(EventArgs e)
        {
            this.updateMaximumSize();

            if (!this.isToolWindow
                && this.WindowState != WindowState.Maximized
                && this.ResizeMode != ResizeMode.CanResize)
            {
                this.ResizeMode = ResizeMode.CanResize;
            }

            base.OnLocationChanged(e);
        }

        protected override void OnStateChanged(EventArgs e)
        {
            this.updateWindowChrome();

            base.OnStateChanged(e);
        }

        #endregion

        #region ui controlling

        private void updateWindowChrome()
        {
            var chrome = new WindowChrome
            {
                CaptionHeight = 40,
                CornerRadius = new CornerRadius(0),
                GlassFrameThickness = new Thickness(1),
                UseAeroCaptionButtons = false
            };

            if (this.isToolWindow)
            {
                chrome.ResizeBorderThickness = new Thickness(0);
            }

            if (this.WindowState == WindowState.Maximized)
            {
                chrome.ResizeBorderThickness = new Thickness(0);
            }

            WindowChrome.SetWindowChrome(this, chrome);
        }
        
        private void updateMaximumSize()
        {
            var screenRect = this.getCurrentScreenRectangle();

            this.MaxWidth = screenRect.Width;
            this.MaxHeight = screenRect.Height;
        }

        private Screen getCurrentScreen()
        {
            return Screen.FromRectangle(new Rectangle(
                (int)this.Left, (int)this.Top,
                (int)this.Width, (int)this.Height
                ));
        }

        protected Rect getCurrentScreenRectangle()
        {
            var screen = this.getCurrentScreen();
            var area = screen.WorkingArea;

            var topleft = new System.Windows.Point(area.Left, area.Top);
            var bottomRight = new System.Windows.Point(area.Right, area.Bottom);

            var presentationSource = PresentationSource.FromVisual(this);
            if (presentationSource != null)
            {
                var compositionTarget = presentationSource.CompositionTarget;
                if (compositionTarget != null)
                {
                    var t = compositionTarget.TransformFromDevice;

                    topleft = t.Transform(topleft);
                    bottomRight = t.Transform(bottomRight);
                }
            }

            return new Rect(topleft, bottomRight);
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
            mmi.ptMinTrackSize.x = (int)this.MinWidth;
            mmi.ptMinTrackSize.y = (int)this.MinHeight;

            Marshal.StructureToPtr(mmi, lParam, true);
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
