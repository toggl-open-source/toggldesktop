
using System;
using System.Windows;
using System.Windows.Input;
using System.Windows.Interop;

namespace TogglDesktop.WPF
{
    public partial class EditViewPopup
    {
        private readonly WindowInteropHelper interopHelper;

        private bool isLeft;
        private bool remainOnTop;
        private bool isResizing;

        public EditViewPopup()
        {
            this.InitializeComponent();
            this.interopHelper = new WindowInteropHelper(this);

            this.MinWidth = this.EditView.MinWidth;

            Toggl.OnTimeEntryEditor += this.onTimeEntryEditor;
        }

        private void onTimeEntryEditor(bool open, Toggl.TogglTimeEntryView te, string focusedFieldName)
        {
            if (this.TryBeginInvoke(this.onTimeEntryEditor, open, te, focusedFieldName))
                return;

            this.Show();
            this.EditView.FocusField(focusedFieldName);
        }

        protected override void OnDeactivated(EventArgs e)
        {
            base.OnDeactivated(e);
            this.SetWindowOnTop(this.remainOnTop);
        }

        public void SetWindowOnTop(bool onTop)
        {
            this.remainOnTop = onTop;
            Win32.SetWindowPos(this.interopHelper.Handle,
                onTop ? Win32.HWND_TOPMOST : Win32.HWND_NOTOPMOST,
                0, 0, 0, 0, Win32.SWP_NOMOVE | Win32.SWP_NOSIZE);
        }

        public void SetPlacement(bool left, double x, double y, double height, bool fixHeight = false)
        {
            this.EditView.SetShadow(left, height);

            this.resizeHandle.HorizontalAlignment = left ? HorizontalAlignment.Left : HorizontalAlignment.Right;

            if (!fixHeight)
                height = Math.Min(700, Math.Max(520, height));

            this.Topmost = true;
            this.isLeft = !left;
            this.Height = height;


            if (left)
            {
                x -= this.Width;
            }

            this.Left = x;
            this.Top = y;

            this.MinWidth = 400;
            this.MinHeight = height;
            this.MaxWidth = 1200;
            this.MaxHeight = height;
        }

        private void onResizeHandleLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            const int htleft = 10;
            const int htright = 11;

            Mouse.Capture(null);

            this.ResizeMode = ResizeMode.CanResize;

            Win32.SendMessage(this.interopHelper.Handle,
                Win32.wmNcLButtonDown,
                this.isLeft ? htright : htleft,
                0);

            this.resizeHandle.CaptureMouse();

            this.isResizing = true;
        }

        private void onResizeHandleLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            this.endResizing();
        }

        private void endResizing()
        {
            if (!this.isResizing)
                return;

            Mouse.Capture(null);
            this.ResizeMode = ResizeMode.NoResize;
            this.isResizing = false;
        }

        private void onWindowMouseMove(object sender, MouseEventArgs e)
        {
            if (e.LeftButton == MouseButtonState.Released)
            {
                this.endResizing();
            }
        }

    }
}
