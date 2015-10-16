using System;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media.Animation;
using System.Windows.Threading;

namespace TogglDesktop
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

        private bool hasSomethingToShow
        {
            get { return Program.IsLoggedIn && (this.unsyncedItems != 0 || this.syncState != Toggl.SyncState.Idle); }
        }

        #region toggl events

        private void onLogin(bool open, ulong userID)
        {
            if (this.TryBeginInvoke(this.onLogin, open, userID))
                return;

            if (open)
            {
                this.Hide();
            }
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

        #endregion

        public void Hide()
        {
            this.Visibility = Visibility.Collapsed;
        }

        private void update()
        {
            if (!this.hasSomethingToShow)
            {
                this.stopSpinnerAnimation();
                this.Hide();
                return;
            }

            if (this.IsVisible)
            {
                this.show();
            }
            else
            {
                this.tryShowingDelayed();
            }
        }

        private void show()
        {
            this.unsyncedCount.Text = this.unsyncedItems == 0
                ? "" : this.unsyncedItems.ToString();

            switch (this.syncState)
            {
                case Toggl.SyncState.Idle:
                    {
                        this.showIdle();
                        break;
                    }
                case Toggl.SyncState.Syncing:
                    {
                        this.showSyncing();
                        break;
                    }
                default:
                    throw new ArgumentOutOfRangeException();
            }

            this.Visibility = Visibility.Visible;
        }

        private async void tryShowingDelayed()
        {
            await Task.Delay(TimeSpan.FromSeconds(3));

            if (this.IsVisible)
                return;

            if (this.hasSomethingToShow)
            {
                this.show();
            }
        }

        private void showIdle()
        {
            this.ToolTip = string.Format("{0} unsynced time entries. Click to Sync.", this.syncState);

            this.stopSpinnerAnimation();
        }

        private void showSyncing()
        {
            this.unsyncedCount.Text = this.unsyncedItems == 0
                ? "" : this.unsyncedItems.ToString();
            this.ToolTip = "Syncing...";

            this.startSpinnerAnimation();
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
