using System;
using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Threading;

namespace TogglDesktop
{
    public partial class MiniTimerWindow
    {
        private readonly ContextMenu contextMenu;
        private readonly WindowInteropHelper interopHelper;

        private bool leftMouseDown;
        private Point mouseDownPosition;
        private bool isResizing;

        public MiniTimerWindow(MainWindow mainWindow)
        {
            this.contextMenu = mainWindow.ContextMenu;
            this.InitializeComponent();
            this.Closing += OnClosing;
            this.WindowStyle = WindowStyle.SingleBorderWindow;

            this.interopHelper = new WindowInteropHelper(this);

            KeyboardShortcuts.RegisterShortcuts(this);
        }

        static void OnClosing(object sender, CancelEventArgs e)
        {
            // disallow closing via Alt+F4
            e.Cancel = true;
        }

        protected override void OnMouseRightButtonDown(MouseButtonEventArgs e)
        {
            this.contextMenu.PlacementTarget = this;
            this.contextMenu.Placement = PlacementMode.Bottom;
            this.contextMenu.HorizontalOffset = 0;
            this.contextMenu.VerticalOffset = 0;

            this.contextMenu.IsOpen = true;

            e.Handled = true;
        }

        protected override void OnLocationChanged(EventArgs e)
        {
            var screenRect = this.GetCurrentScreenRectangle();

            this.snapToEdges(screenRect);

            base.OnLocationChanged(e);
        }

        private void snapToEdges(Rect screenRect)
        {
            const double snapLimit = 10;

            if (Math.Abs(this.Left - screenRect.Left) < snapLimit)
            {
                this.Left = screenRect.Left;
            }
            else if (Math.Abs(this.Left + this.ActualWidth - screenRect.Right) < snapLimit)
            {
                this.Left = screenRect.Right - this.ActualWidth;
            }

            if (Math.Abs(this.Top - screenRect.Top) < snapLimit)
            {
                this.Top = screenRect.Top;
            }
            else if (Math.Abs(this.Top + this.ActualHeight - screenRect.Bottom) < snapLimit)
            {
                this.Top = screenRect.Bottom - this.ActualHeight;
            }
        }

        protected override void OnPreviewMouseLeftButtonDown(MouseButtonEventArgs e)
        {
            this.leftMouseDown = true;
            this.mouseDownPosition = e.GetPosition(this);
        }

        protected override void OnPreviewMouseMove(MouseEventArgs e)
        {
            if (this.leftMouseDown)
            {
                if (e.LeftButton != MouseButtonState.Pressed)
                {
                    this.leftMouseDown = false;
                    return;
                }

                var mousePosition = e.GetPosition(this);
                if (mousePosition == this.mouseDownPosition)
                {
                    return;
                }

                this.Dispatcher.BeginInvoke(DispatcherPriority.Normal, new Action(this.startDrag));
            }
        }

        private void startDrag()
        {
            if (Mouse.LeftButton != MouseButtonState.Pressed)
            {
                this.leftMouseDown = false;
                return;
            }

            this.DragMove();
        }

        public void SetManualMode(bool isInManualMode)
        {
            this.timer.SetManualMode(isInManualMode);
        }

        public void SetVisible(bool visible)
        {
            if (visible)
            {
                this.Show();
            }
            else
            {
                this.Hide();
            }
        }


        #region resizing

        private void onLeftResizeHandleButtonDown(object sender, MouseButtonEventArgs e)
        {
            this.startResizing(true, (FrameworkElement)sender);
        }

        private void onRightResizeHandleButtonDown(object sender, MouseButtonEventArgs e)
        {
            this.startResizing(false, (FrameworkElement)sender);
        }

        private void onResizeHandleButtonUp(object sender, MouseButtonEventArgs e)
        {
            this.endResizing();
        }

        private void onWindowMouseMove(object sender, MouseEventArgs e)
        {
            if (e.LeftButton == MouseButtonState.Released)
            {
                this.endResizing();
            }
        }

        private void startResizing(bool left, FrameworkElement handle)
        {
            if (this.isResizing)
                return;

            const int htleft = 10;
            const int htright = 11;

            Mouse.Capture(null);

            this.ResizeMode = ResizeMode.CanResize;

            Win32.SendMessage(this.interopHelper.Handle,
                Win32.wmNcLButtonDown,
                left ? htleft : htright,
                0);

            handle.CaptureMouse();

            this.isResizing = true;
        }

        private void endResizing()
        {
            if (!this.isResizing)
                return;

            Mouse.Capture(null);
            this.ResizeMode = ResizeMode.NoResize;
            this.isResizing = false;
        }

        #endregion

    }
}
