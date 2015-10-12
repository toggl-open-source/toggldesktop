
using System;
using System.Windows;
using System.Windows.Media.Animation;

namespace TogglDesktop.WPF
{
    public partial class SyncingIndicator
    {
        private readonly Storyboard spinnerAnimation;

        private long unsyncedItems;
        private Toggl.SyncState syncState;
        private bool spinnerIsAnimating;

        public SyncingIndicator()
        {
            this.InitializeComponent();

            this.spinnerAnimation = (Storyboard)this.Resources["RotateSpinner"];

            Toggl.OnDisplaySyncState += this.onDisplaySyncState;
            Toggl.OnDisplayUnsyncedItems += this.onDisplayUnsyncedItems;
            Toggl.OnLogin += this.onLogin;
        }


        private void onLogin(bool open, ulong userID)
        {
            if (this.TryBeginInvoke(this.onLogin, open, userID))
                return;

            if (open)
            {
                this.Hide();
            }
        }

        public void Hide()
        {
            this.Visibility = Visibility.Collapsed;
        }

        private void onDisplayUnsyncedItems(long count)
        {
            if (this.TryBeginInvoke(this.onDisplayUnsyncedItems, count))
                return;

            this.unsyncedItems = count;
            this.update();
        }

        private void onDisplaySyncState(Toggl.SyncState state)
        {
            if (this.TryBeginInvoke(this.onDisplaySyncState, state))
                return;

            this.syncState = state;
            this.update();
        }

        private void update()
        {
            if (!Program.IsLoggedIn)
            {
                this.stopSpinnerAnimation();
                this.Hide();
                return;
            }

            switch (this.syncState)
            {
                case Toggl.SyncState.Idle:
                {
                    this.stopSpinnerAnimation();

                    if (this.unsyncedItems == 0)
                    {
                        this.Hide();
                        return;
                    }
                    this.ToolTip = string.Format("{0} unsynced time entries. Click to Sync.", this.syncState);
                    break;
                }
                case Toggl.SyncState.Syncing:
                {
                    this.startSpinnerAnimation();
                    this.ToolTip = "Syncing...";
                    break;
                }
                default:
                    throw new ArgumentOutOfRangeException();
            }
            this.unsyncedCount.Text = this.unsyncedItems == 0
                ? "" : this.unsyncedItems.ToString();
            this.Visibility = Visibility.Visible;
        }

        private void stopSpinnerAnimation()
        {
            if (!this.spinnerIsAnimating)
                return;

            this.spinnerAnimation.Stop();
            this.spinnerIsAnimating = false;
            this.syncButton.IsEnabled = true;
        }

        private void startSpinnerAnimation()
        {
            if (this.spinnerIsAnimating)
                return;

            this.spinnerAnimation.Begin();
            this.spinnerIsAnimating = true;
            this.syncButton.IsEnabled = false;
        }

        private void onSyncButtonClick(object sender, RoutedEventArgs e)
        {
            Toggl.Sync();
        }
    }
}
