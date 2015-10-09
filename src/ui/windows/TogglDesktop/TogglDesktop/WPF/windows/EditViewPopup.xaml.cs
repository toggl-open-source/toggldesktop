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

            if (!this.Owner.IsVisible)
                return;

            this.Show();
            this.EditView.FocusField(focusedFieldName);
        }

        public void ClosePopup()
        {
            this.EditView.EnsureSaved();
            this.Hide();
        }

        #region ui events

        private void onResizeHandleLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            this.startResizing();
        }

        private void onResizeHandleLeftButtonUp(object sender, MouseButtonEventArgs e)
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

        #endregion

        #region controlling

        public void SetPlacement(bool left,
            double x, double y, double height, double maxWidth, bool fixHeight = false)
        {
            this.setShadow(left ^ fixHeight, height);

            this.resizeHandle.HorizontalAlignment = left ? HorizontalAlignment.Left : HorizontalAlignment.Right;

            if (!fixHeight)
                height = Math.Min(700, Math.Max(520, height));

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
            this.MaxWidth = maxWidth;
            this.MaxHeight = height;
        }

        private void setShadow(bool left, double height)
        {
            this.mainFormShadow.Height = height;
            this.mainFormShadow.HorizontalAlignment = left ? HorizontalAlignment.Right : HorizontalAlignment.Left;
        }

        private void startResizing()
        {
            if (this.isResizing)
                return;

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
