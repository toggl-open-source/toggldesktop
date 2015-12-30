

using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using System.Windows.Threading;

namespace TogglDesktop
{
    public partial class MiniTimerWindow
    {
        private readonly ContextMenu contextMenu;

        private bool leftMouseDown;
        private Point mouseDownPosition;

        public MiniTimerWindow(ContextMenu contextMenu)
        {
            this.contextMenu = contextMenu;
            this.InitializeComponent();
            this.WindowStyle = WindowStyle.SingleBorderWindow;
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
            var screenRect = this.getCurrentScreenRectangle();

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
    }
}
