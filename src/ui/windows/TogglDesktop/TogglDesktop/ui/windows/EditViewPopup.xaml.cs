using System;
using System.ComponentModel;
using System.Windows;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media.Animation;
using TogglDesktop.Diagnostics;

namespace TogglDesktop
{
    public partial class EditViewPopup
    {
        enum AnimationStates
        {
            Opening,
            Closing,
        }

        private AnimationStates animationState = AnimationStates.Closing;

        private readonly WindowInteropHelper interopHelper;

        private bool isLeft;
        private bool isResizing;
        private bool skipAnimation;
        private object animationToken;

        public EditViewPopup()
        {
            this.InitializeComponent();
            this.interopHelper = new WindowInteropHelper(this);

            this.MinWidth = this.EditView.MinWidth;
            this.mainGrid.Width = 0;

            Toggl.OnTimeEntryEditor += this.onTimeEntryEditor;
        }

        private void onTimeEntryEditor(bool open, Toggl.TogglTimeEntryView te, string focusedFieldName)
        {
            if (this.TryBeginInvoke(this.onTimeEntryEditor, open, te, focusedFieldName))
                return;

            if (!this.Owner.IsVisible)
                return;
            
            using (Performance.Measure("opening edit popup"))
            {
                if (this.skipAnimation)
                {
                    this.stopAnimationOpen();
                }
                else
                {
                    this.startAnimationOpen();
                }

                this.Show();
                this.EditView.FocusField(focusedFieldName);
            }
        }

        public void ClosePopup(bool skipAnimation = false)
        {
            if (skipAnimation || this.skipAnimation)
            {
                this.EditView.EnsureSaved();
                this.stopAnimationClose();
            }
            else
            {
                if (this.animationState == AnimationStates.Closing)
                    return;

                this.EditView.EnsureSaved();
                this.startAnimationClose();
            }
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

        protected override void OnClosing(CancelEventArgs e)
        {
            this.ClosePopup();
            e.Cancel = true;
        }

        #endregion

        #region animate

        private void startAnimationOpen()
        {
            if (this.animationState == AnimationStates.Opening)
                return;

            this.animationState = AnimationStates.Opening;

            this.setAlignmentsForAnimation();
            this.mainGrid.Width = 0;
            this.EditView.Width = this.ActualWidth;
            this.mainGrid.IsEnabled = true;

            this.startAnimation(0, this.ActualWidth, this.stopAnimationOpen);
        }

        private void stopAnimationOpen()
        {
            this.animationToken = null;

            this.animationState = AnimationStates.Opening;

            this.mainGrid.BeginAnimation(WidthProperty, null);

            this.mainGrid.Width = double.NaN;
            this.EditView.Width = double.NaN;
            this.mainGrid.HorizontalAlignment = HorizontalAlignment.Stretch;
            this.EditView.HorizontalAlignment = HorizontalAlignment.Stretch;
            this.mainGrid.IsEnabled = true;
        }

        private void startAnimationClose()
        {
            if (this.animationState == AnimationStates.Closing)
                return;

            this.animationState = AnimationStates.Closing;

            this.setAlignmentsForAnimation();
            this.EditView.Width = this.ActualWidth;
            this.mainGrid.IsEnabled = false;

            this.startAnimation(this.ActualWidth, 0, this.stopAnimationClose);
        }

        private void stopAnimationClose()
        {
            this.animationToken = null;

            this.animationState = AnimationStates.Closing;

            this.mainGrid.BeginAnimation(WidthProperty, null);

            this.mainGrid.Width = 0;

            this.Hide();
        }

        private void startAnimation(double from, double to, Action stopAction)
        {
            var token = new object();
            this.animationToken = token;

            var animation = new DoubleAnimation(from, to,
               new Duration(TimeSpan.FromSeconds(0.15)),
               FillBehavior.HoldEnd);
            animation.Completed += (s, e) =>
            {
                if (token != this.animationToken)
                {
                    this.animationToken = null;
                    return;
                }
                stopAction();
            };

            this.mainGrid.BeginAnimation(WidthProperty, animation);
        }

        private void setAlignmentsForAnimation()
        {
            if (this.isLeft)
            {
                this.mainGrid.HorizontalAlignment = HorizontalAlignment.Left;
                this.EditView.HorizontalAlignment = HorizontalAlignment.Right;
            }
            else
            {
                this.mainGrid.HorizontalAlignment = HorizontalAlignment.Right;
                this.EditView.HorizontalAlignment = HorizontalAlignment.Left;
            }
        }

        #endregion

        #region controlling

        public void SetPlacement(bool left,
            double x, double y, double height, double maxWidth, bool fixHeight = false)
        {
            this.setShadow(left ^ fixHeight, height);

            this.resizeHandle.HorizontalAlignment = left ? HorizontalAlignment.Left : HorizontalAlignment.Right;

            this.skipAnimation = fixHeight;

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

            this.MinHeight = height;
            this.MaxHeight = height;

            this.MaxWidth = maxWidth;
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
