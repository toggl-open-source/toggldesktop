using System;
using System.ComponentModel;
using System.Reactive.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using System.Windows.Threading;

namespace TogglDesktop
{
    public partial class MiniTimerWindow
    {
        private bool leftMouseDown;
        private Point mouseDownPosition;

        public MiniTimerWindow(MainWindow mainWindow)
        {
            this.InitializeComponent();
            MainContextMenu.DataContext = mainWindow.CogwheelMenuViewModel;
            MainContextMenu.Initialize(mainWindow);
            this.Closing += OnClosing;
            var sizeChangedObservable = Observable.FromEventPattern<SizeChangedEventHandler, SizeChangedEventArgs>(
                handler => this.SizeChanged += handler,
                handler => this.SizeChanged -= handler);
            sizeChangedObservable
                .Where(x => x.EventArgs.WidthChanged)
                .Throttle(TimeSpan.FromMilliseconds(500))
                .ObserveOnDispatcher()
                .Subscribe(x => ((Window) x.Sender).SizeToContent = SizeToContent.Height);

            this.timer.MouseCaptured += OnMouseCaptured;

            KeyboardShortcuts.RegisterShortcuts(this);
        }

        static void OnClosing(object sender, CancelEventArgs e)
        {
            // disallow closing via Alt+F4
            e.Cancel = true;
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

            var rootGridMargin = this.rootGrid.Margin;

            var miniTimerLeft = this.Left + rootGridMargin.Left;
            var miniTimerWidth = this.ActualWidth - rootGridMargin.Left - rootGridMargin.Right;

            if (Math.Abs(miniTimerLeft - screenRect.Left) < snapLimit)
            {
                this.Left = screenRect.Left - rootGridMargin.Left;
            }
            else if (Math.Abs(miniTimerLeft + miniTimerWidth - screenRect.Right) < snapLimit)
            {
                this.Left = screenRect.Right - this.ActualWidth + rootGridMargin.Right;
            }

            var miniTimerTop = this.Top + rootGridMargin.Top;
            var miniTimerHeight = this.ActualHeight - rootGridMargin.Top - rootGridMargin.Bottom;

            if (Math.Abs(miniTimerTop - screenRect.Top) < snapLimit)
            {
                this.Top = screenRect.Top - rootGridMargin.Top;
            }
            else if (Math.Abs(miniTimerTop + miniTimerHeight - screenRect.Bottom) < snapLimit)
            {
                this.Top = screenRect.Bottom - this.ActualHeight + rootGridMargin.Bottom;
            }
        }

        private void OnMouseCaptured(object sender, MouseButtonEventArgs e)
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
