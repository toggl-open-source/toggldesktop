
using System;
using System.Windows;
using System.Windows.Media.Animation;

namespace TogglDesktop.WPF
{
    public partial class StatusBar
    {
        private readonly Storyboard spinnerAnimation;

        private Toggl.OnlineState onlineState;
        private long unsyncedItems;
        private Toggl.SyncState syncState;
        private bool spinnerIsAnimating;

        public StatusBar()
        {
            this.InitializeComponent();

            this.spinnerAnimation = (Storyboard)this.Resources["RotateSpinner"];

            Toggl.OnDisplaySyncState += this.onDisplaySyncState;
            Toggl.OnDisplayUnsyncedItems += this.onDisplayUnsyncedItems;
            Toggl.OnOnlineState += this.onOnlineState;
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

        private void onOnlineState(Toggl.OnlineState state)
        {
            if (this.TryBeginInvoke(this.onOnlineState, state))
                return;

            this.onlineState = state;
            this.update();
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

            if (this.onlineState != Toggl.OnlineState.Online)
            {
                this.stopSpinnerAnimation();
                this.statusText.Text = "Offline";
                this.syncButton.Visibility = Visibility.Collapsed;
                this.Visibility = Visibility.Visible;
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
                    this.statusText.Text = "Un-synced entries: " + this.unsyncedItems;
                    break;
                }
                case Toggl.SyncState.Syncing:
                {
                    this.startSpinnerAnimation();

                    this.statusText.Text = "Syncing...";
                    break;
                }
                default:
                    throw new ArgumentOutOfRangeException();
            }
            this.syncButton.Visibility = Visibility.Visible;
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

        public void Hide()
        {
            this.Visibility = Visibility.Collapsed;
        }
    }
}
