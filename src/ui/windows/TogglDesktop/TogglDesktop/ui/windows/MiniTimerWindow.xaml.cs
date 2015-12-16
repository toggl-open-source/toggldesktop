

using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Threading;

namespace TogglDesktop
{
    public partial class MiniTimerWindow
    {
        private bool leftMouseDown;
        private Point mouseDownPosition;

        public MiniTimerWindow(ContextMenu contextMenu)
        {
            this.ContextMenu = contextMenu;
            this.InitializeComponent();
            this.WindowStyle = WindowStyle.SingleBorderWindow;
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
    }
}
